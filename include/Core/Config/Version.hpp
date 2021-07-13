#pragma once

#include <string>

namespace obe::Config
{
    struct Version
    {
        int version_major = 0;
        int version_minor = 0;
        int version_patch = 0;

        constexpr Version(int version_major, int version_minor, int version_patch)
            : version_major(version_major)
            , version_minor(version_minor)
            , version_patch(version_patch) {};
        Version(const std::string& str);
    };

    inline constexpr bool operator==(const Version& lhs, const Version& rhs)
    {
        return (lhs.version_major == rhs.version_major
            && lhs.version_minor == rhs.version_minor
            && lhs.version_patch == rhs.version_patch);
    }

    inline constexpr bool operator!=(const Version& lhs, const Version& rhs)
    {
        return !(lhs == rhs);
    }

    inline constexpr bool operator<(const Version& lhs, const Version& rhs)
    {
        if (lhs.version_major < rhs.version_major)
            return true;
        if (lhs.version_major > rhs.version_major)
            return false;
        if (lhs.version_minor < rhs.version_minor)
            return true;
        if (lhs.version_minor > rhs.version_minor)
            return false;
        return (lhs.version_patch < rhs.version_patch);
    }

    inline constexpr bool operator<=(const Version& lhs, const Version& rhs)
    {
        return (lhs < rhs) || (lhs == rhs);
    }

    inline constexpr bool operator>(const Version& lhs, const Version& rhs)
    {
        return rhs < lhs;
    }

    inline constexpr bool operator>=(const Version& lhs, const Version& rhs)
    {
        return (lhs > rhs) || (lhs == rhs);
    }
} // namespace obe::Config
