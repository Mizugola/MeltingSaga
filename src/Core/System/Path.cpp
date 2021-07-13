#include <System/Path.hpp>
#include <Utils/StringUtils.hpp>

namespace obe::System
{
    std::string joinPath(const std::string& base, const std::string& path)
    {
        return base + ((!base.empty()) ? "/" : "") + path;
    }

    std::pair<std::string, std::string> splitPathAndPrefix(
        const std::string& path, bool warnOnMissingPrefix)
    {
        if (const auto prefixPos = path.find("://"); prefixPos != std::string::npos)
        {
            std::string prefix = path.substr(0, prefixPos);
            if (prefix == "*")
            {
                prefix.clear();
            }
            return std::make_pair(path.substr(prefixPos + 3), prefix);
        }

        if (warnOnMissingPrefix)
        {
            Debug::Log->warn("Path '{}' has no prefix, defaulting to '*://'", path);
        }
        return std::make_pair(path, "");
    }

    std::vector<MountablePath> filterMountablePathsWithPrefix(const std::vector<MountablePath>& mounts, const std::string& prefix)
    {
        std::vector<MountablePath> validMounts;
        for (const MountablePath& mountablePath : mounts)
        {
            if (mountablePath.prefix == prefix || (prefix.empty() && mountablePath.implicit))
            {
                validMounts.push_back(mountablePath);
            }
        }
        if (validMounts.empty())
        {
            std::vector<std::string> allPrefixes;
            allPrefixes.reserve(mounts.size());
            std::transform(mounts.begin(), mounts.end(), std::back_inserter(allPrefixes),
                [](const MountablePath& mount) { return mount.prefix; });
            throw Exceptions::UnknownPathPrefix(prefix, allPrefixes, EXC_INFO);
        }
        return validMounts;
    }

    std::unordered_map<std::string, FindResult> Path::PathCache;

    void FindResult::checkValidity() const
    {
        if (!success())
        {
            std::vector<std::string> mountsAsStrings;
            std::transform(m_mounts.begin(), m_mounts.end(),
                std::back_inserter(mountsAsStrings), [](const MountablePath& mount) {
                    return fmt::format("\"{}://{}\"", mount.prefix, mount.basePath);
                });
            throw Exceptions::ResourceNotFound(m_path, mountsAsStrings, EXC_INFO);
        }
    }

    FindResult::FindResult(const std::string& pathNotFound, const std::vector<MountablePath>& mounts)
    {
        m_path = pathNotFound;
        m_mounts = mounts;
    }

    FindResult::FindResult(PathType pathType, const std::string& root,
        const std::string& path, const std::string& element)
    {
        m_type = pathType;
        m_root = root;
        m_path = path;
        m_element = element;
    }

    const std::string& FindResult::path() const
    {
        checkValidity();
        return m_path;
    }

    const std::string& FindResult::root() const
    {
        checkValidity();
        return m_root;
    }

    const std::string& FindResult::element() const
    {
        return m_element;
    }

    bool FindResult::success() const
    {
        return m_type != PathType::All;
    }

    FindResult::operator bool() const
    {
        return this->success();
    }

    FindResult::operator const std::string &() const
    {
        checkValidity();
        return m_path;
    }

    FindResult::operator const char*() const
    {
        checkValidity();
        return m_path.c_str();
    }

    FindResult::operator std::string_view() const
    {
        checkValidity();
        return m_path;
    }

    Path::Path()
        : m_mounts(MountablePath::Paths())
    {
    }

    Path::Path(const std::vector<MountablePath>& mount)
        : m_mounts(mount)
    {
    }

    Path::Path(const Path& path)
        : m_mounts(path.m_mounts)
    {
        this->m_path = path.toString();
        this->m_prefix = path.m_prefix;
    }

    Path::Path(std::string_view path)
        : m_mounts(MountablePath::Paths())
    {
        std::tie(m_path, m_prefix) = splitPathAndPrefix(path.data());
    }

    Path::Path(std::string_view prefix, std::string_view path) : m_mounts(MountablePath::Paths())
    {
        m_prefix = prefix;
        m_path = path;
    }

    Path& Path::set(const std::string& path)
    {
        std::tie(m_path, m_prefix) = splitPathAndPrefix(path);
        return *this;
    }

    Path Path::add(const std::string& path) const
    {
        return Path(m_prefix,
            m_path + ((!m_path.empty() && m_path.back() != '/') ? "/" : "") + path);
    }

    std::string Path::last() const
    {
        std::vector<std::string> splitPath = Utils::String::split(m_path, "/");
        return splitPath.back();
    }

    Path Path::getPath(const std::size_t index) const
    {
        if (index < m_mounts.size())
            return Path(m_mounts[index].basePath).add(m_path);
        std::vector<std::string> mountNames;
        mountNames.reserve(m_mounts.size());
        for (const auto& mount : m_mounts)
        {
            mountNames.push_back(mount.basePath);
        }
        throw Exceptions::MountablePathIndexOverflow(
            index, m_mounts.size(), mountNames, EXC_INFO);
    }

    std::vector<FindResult> Path::list(PathType pathType) const
    {
        std::vector<FindResult> results;
        const std::vector<MountablePath> validMounts
            = filterMountablePathsWithPrefix(m_mounts, m_prefix);
        for (const MountablePath& mountedPath : validMounts)
        {
            std::string fullPath = joinPath(mountedPath.basePath, m_path);

            if (Utils::File::directoryExists(fullPath))
            {
                if (pathType == PathType::All || pathType == PathType::Directory)
                {
                    std::vector<std::string> directories
                        = Utils::File::getDirectoryList(fullPath);
                    for (const std::string& directory : directories)
                    {
                        results.emplace_back(PathType::Directory, mountedPath.basePath,
                            joinPath(fullPath, directory), directory);
                    }
                }
                else if (pathType == PathType::All || pathType == PathType::File)
                {
                    std::vector<std::string> files = Utils::File::getFileList(fullPath);
                    for (const std::string& file : files)
                    {
                        results.emplace_back(PathType::File, mountedPath.basePath,
                            joinPath(fullPath, file), file);
                    }
                }
            }
        }

        return results;
    }

    FindResult Path::find(PathType pathType) const
    {
        if (const auto cacheResult = PathCache.find(m_path);
            cacheResult != PathCache.end())
        {
            return cacheResult->second;
        }
        const std::vector<MountablePath> validMounts
            = filterMountablePathsWithPrefix(m_mounts, m_prefix);
        for (const MountablePath& mountedPath : validMounts)
        {
            const std::string fullPath = mountedPath.basePath
                + ((!mountedPath.basePath.empty()) ? "/" : "") + m_path;
            if ((pathType == PathType::All || pathType == PathType::File)
                && Utils::File::fileExists(fullPath))
            {
                const std::string result = joinPath(mountedPath.basePath, m_path);
                return PathCache
                    .emplace(m_path,
                        FindResult(PathType::File, mountedPath.basePath, result, m_path))
                    .first->second;
            }
            else if ((pathType == PathType::All || pathType == PathType::Directory)
                && Utils::File::directoryExists(fullPath))
            {
                const std::string result = joinPath(mountedPath.basePath, m_path);
                return PathCache
                    .emplace(m_path,
                        FindResult(
                            PathType::Directory, mountedPath.basePath, result, m_path))
                    .first->second;
            }
        }
        return FindResult(m_path, validMounts);
    }

    std::vector<FindResult> Path::findAll(PathType pathType) const
    {
        std::vector<FindResult> results;
        const std::vector<MountablePath> validMounts
            = filterMountablePathsWithPrefix(m_mounts, m_prefix);
        for (const MountablePath& mountedPath : validMounts)
        {
            const std::string fullPath = joinPath(mountedPath.basePath, m_path);
            if ((pathType == PathType::All || pathType == PathType::File)
                && Utils::File::fileExists(fullPath))
            {
                results.emplace_back(
                    PathType::File, mountedPath.basePath, fullPath, m_path);
            }
            else if ((pathType == PathType::All || pathType == PathType::Directory)
                && Utils::File::directoryExists(fullPath))
            {
                results.emplace_back(
                    PathType::Directory, mountedPath.basePath, fullPath, m_path);
            }
        }
        return results;
    }

    std::string Path::toString() const
    {
        return m_path;
    }

    Path& Path::operator=(const Path& path)
    {
        m_path = path.m_path;
        m_prefix = path.m_prefix;

        return *this;
    }

    ContextualPathFactory::ContextualPathFactory(const std::string& base)
    {
        m_mounts = System::MountablePath::Paths();
        m_mounts.push_back(
            System::MountablePath(System::MountablePathType::Path, base, "self"));
    }

    ContextualPathFactory::ContextualPathFactory(const std::string& base,
        const std::vector<MountablePath>& customMounts)
    {
        m_mounts = customMounts;
        m_mounts.push_back(
            System::MountablePath(System::MountablePathType::Path, base, "self"));
    }

    Path ContextualPathFactory::operator()(const std::string& path) const
    {
        return System::Path(m_mounts).set(path);
    }
} // namespace obe::System

obe::System::Path operator"" _path(const char* str, std::size_t len)
{
    return obe::System::Path(std::string(str, len));
}

std::string operator""_fs(const char* str, std::size_t len)
{
    return obe::System::Path(std::string(str, len)).find();
}
