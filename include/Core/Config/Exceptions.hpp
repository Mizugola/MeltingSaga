#include <Exception.hpp>

#include <string_view>

namespace obe::Config::Exceptions
{
    class InvalidVersionFormat : public Exception
    {
    public:
        InvalidVersionFormat(std::string_view detail, DebugInfo info)
            : Exception("InvalidVersionNumber", info)
        {
            this->error(
                "There is a problem with your version number : {}", detail);
        }
    };
}
