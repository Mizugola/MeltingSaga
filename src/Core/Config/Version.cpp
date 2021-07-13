#include <Config/Exceptions.hpp>
#include <Config/Version.hpp>

#include <Utils/StringUtils.hpp>

#include <string>

namespace obe::Config
{
    Version::Version(const std::string& str)
    {
        std::vector<std::string> parts = Utils::String::split(str, ".");
        if (parts.size() != 3)
        {
            throw Exceptions::InvalidVersionFormat(
                "Wrong number of components for version string. Expecting version "
                "string of the form v?MAJOR.MINOR.PATCH",
                EXC_INFO);
        }
        // Skip first letter from version string ( 'v' )
        if (!Utils::String::isStringNumeric(parts[0]) && parts[0].size() >= 2)
        {
            if (parts[0][0] != 'v') {
                throw Exceptions::InvalidVersionFormat(
                "Wrong prefix for version string, only \"v0.1.1\" is allowed.",
                EXC_INFO);
            }
            parts[0] = parts[0].substr(1);
        }

        version_major = std::stoi(parts[0]);
        version_minor = std::stoi(parts[1]);
        version_patch = std::stoi(parts[2]);
    }
} // namespace obe::Config
