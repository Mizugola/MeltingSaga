#include <Debug/Logger.hpp>
#include <Utils/FileUtils.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#if defined(_WIN32) || defined(_WIN64)
#include <spdlog/sinks/wincolor_sink.h>
#elif defined(__ANDROID__)
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/ansicolor_sink.h>
#endif

namespace obe::Debug
{
    std::shared_ptr<spdlog::logger> Log;
    void InitLogger()
    {
        Utils::File::deleteFile("debug.log");
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_st>();

        const auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
        const auto sink2
            = std::make_shared<spdlog::sinks::basic_file_sink_st>("debug.log");

        dist_sink->add_sink(sink1);
        dist_sink->add_sink(sink2);
        Log = std::make_shared<spdlog::logger>("Log", dist_sink);
        Log->set_pattern("[%H:%M:%S.%e]<%^%l%$> : %v");
        Log->set_level(spdlog::level::info);
        Log->flush_on(spdlog::level::warn);
    }

    void trace(const std::string& content)
    {
        Log->trace(content);
    }

    void debug(const std::string& content)
    {
        Log->debug(content);
    }

    void info(const std::string& content)
    {
        Log->info(content);
    }

    void warn(const std::string& content)
    {
        Log->warn(content);
    }

    void error(const std::string& content)
    {
        Log->error(content);
    }

    void critical(const std::string& content)
    {
        Log->critical(content);
    }
} // namespace obe::Debug
