#pragma once

#include <string>
#include <vector>

namespace obe::System
{
    /**
     * \brief Defines the source of a mounted path
     * \bind{MountablePathType}
     */
    enum class MountablePathType
    {
        /**
         * \brief A standard path
         */
        Path,
        /**
         * \brief The mounted path is a Package
         */
        Package,
        /**
         * \brief The mounted path is a Workspace
         */
        Workspace
    };

    /**
     * \brief Class used to encapsulate mounted Paths
     */
    class MountablePath
    {
    private:
        static std::vector<MountablePath> MountedPaths;

    public:
        /**
         * \brief Constructor of MountablePath
         * \param pathType Type of the mounted path
         * \param basePath Path to the mounted path
         * \param priority Priority of the mounted path
         */
        MountablePath(MountablePathType pathType, const std::string& basePath, const std::string& prefix,
            unsigned int priority = 0, bool implicit = true);
        /**
         * \brief Type of the mounted path
         */
        MountablePathType pathType;
        /**
         * \brief Path of the mounted path
         */
        std::string basePath;
        /**
         * \brief Prefix of the mounted path
         */
        std::string prefix;
        /**
         * \brief Priority of the mounted path (Higher priority means overriding
         *        lower priority Paths)
         */
        unsigned int priority;
        /**
         * \brief Allows the path to be used implicitly (without prefix)
         */
        bool implicit;

        bool operator==(const MountablePath& other) const;

    public:
        /**
         * \brief Function called to Mount all Paths using 'Mount.vili' file
         */
        static void LoadMountFile();
        /**
         * \brief Add a Path to Mounted Paths
         * \param path Path to mount
         */
        static void Mount(MountablePath path);
        /**
         * \brief Remove a Path from Mounted Paths
         * \param path Path to unmount
         */
        static void Unmount(MountablePath path);
        /**
         * \brief Remove all Paths from Mounted Paths
         */
        static void UnmountAll();
        /**
         * \brief All the Mounted Paths
         */
        static const std::vector<MountablePath>& Paths();
        /**
         * \brief All the Mounted Paths as strings
         */
        static std::vector<std::string> StringPaths();
        /**
         * \brief Sort the mounted paths based on their priorities
         */
        static void Sort();
    };
} // namespace obe::System