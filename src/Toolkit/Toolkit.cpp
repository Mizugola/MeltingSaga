#include <functional>
#include <iostream>
#include <memory>

#include <sol/sol.hpp>
#include <styler/styler.hpp>

#include <Debug/Logger.hpp>
#include <Graphics/Color.hpp>
#include <Script/Scripting.hpp>
#include <System/Path.hpp>

namespace obe::Bindings
{
    void IndexAllBindings(sol::state_view state);
}

int lua_exception_handler2(lua_State* L,
    sol::optional<const std::exception&> maybe_exception, sol::string_view description)
{
    if (maybe_exception)
    {
        const std::exception& ex = *maybe_exception;
        obe::Debug::Log->error("<LuaError>[Exception] : {}", ex.what());
    }
    else
    {
        obe::Debug::Log->error("<LuaError>[Error] : {}", description);
    }
    return sol::stack::push(L, description);
}

styler::Foreground convertColor(obe::Graphics::Color color)
{
    if (color.toName() == "white")
    {
        return styler::Foreground::White;
    }
    else if (color.toName() == "red")
    {
        return styler::Foreground::Red;
    }
    else if (color.toName() == "green")
    {
        return styler::Foreground::Green;
    }
    else if (color.toName() == "blue")
    {
        return styler::Foreground::Blue;
    }
    else if (color.toName() == "yellow")
    {
        return styler::Foreground::Yellow;
    }
    else if (color.toName() == "magenta")
    {
        return styler::Foreground::Magenta;
    }
    else if (color.toName() == "cyan")
    {
        return styler::Foreground::Cyan;
    }
    else if (color.toName() == "black")
    {
        return styler::Foreground::Black;
    }
    else
    {
        return styler::Foreground::White;
    }
}

void run(const std::string& command)
{
    using namespace obe;

    Debug::InitLogger();
    Debug::Log->set_level(spdlog::level::debug);
    System::MountablePath::LoadMountFile();

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table,
        sol::lib::package, sol::lib::os, sol::lib::coroutine, sol::lib::math,
        sol::lib::count, sol::lib::debug, sol::lib::io, sol::lib::bit32);

    lua.safe_script("LuaCore = {}");
    lua.safe_script_file("obe://Lib/Internal/ScriptInit.lua"_fs);

    Bindings::IndexAllBindings(lua);
    lua.safe_script_file("obe://Lib/Internal/Searcher.lua"_fs);
    lua.safe_script_file("obe://Lib/Internal/Logger.lua"_fs);
    lua.set_exception_handler(&lua_exception_handler2);

    lua["_term_display"]
        = [](std::vector<std::string> texts, std::vector<obe::Graphics::Color> colors) {
              for (size_t i = 0; i < texts.size(); i++)
              {
                  const std::string_view text = texts[i];
                  const Graphics::Color color = colors[i];
                  std::cout << convertColor(color) << text;
              }
              std::cout << std::endl;
          };

    lua.safe_script_file("obe://Lib/Toolkit/Toolkit.lua"_fs);

    Script::safeLuaCall(lua["evaluate"].get<sol::protected_function>(), command);
}

int main(int argc, char** argv)
{
    std::string command;
    std::for_each(argv + 1, argv + argc,
        [&](const char* c_str) { command += std::string(c_str) + " "; });

#if defined _DEBUG
    run(command);
#else
    try
    {
        run(command);
    }
    catch (const std::exception& e)
    {
        if (Debug::Log)
        {
            Debug::Log->error("The following error occurred while running obey");
            Debug::Log->error(e.what());
        }
        else
        {
            std::cerr << "The following error occurred while running obey" << std::endl;
            std::cerr << e.what() << std::endl;
        }
        return 1;
    }
#endif

    return 0;
}
