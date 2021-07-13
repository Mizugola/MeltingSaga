#include <Bindings/obe/Utils/File/File.hpp>

#include <Utils/FileUtils.hpp>

#include <Bindings/Config.hpp>

namespace obe::Utils::File::Bindings
{
    void LoadFunctionGetDirectoryList(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function(
            "getDirectoryList", obe::Utils::File::getDirectoryList);
    }
    void LoadFunctionGetFileList(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("getFileList", obe::Utils::File::getFileList);
    }
    void LoadFunctionFileExists(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("fileExists", obe::Utils::File::fileExists);
    }
    void LoadFunctionDirectoryExists(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("directoryExists", obe::Utils::File::directoryExists);
    }
    void LoadFunctionCreateDirectory(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("createDirectory", obe::Utils::File::createDirectory);
    }
    void LoadFunctionCreateFile(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("createFile", obe::Utils::File::createFile);
    }
    void LoadFunctionCopy(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("copy", obe::Utils::File::copy);
    }
    void LoadFunctionDeleteFile(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("deleteFile", obe::Utils::File::deleteFile);
    }
    void LoadFunctionDeleteDirectory(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("deleteDirectory", obe::Utils::File::deleteDirectory);
    }
    void LoadFunctionGetCurrentDirectory(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function(
            "getCurrentDirectory", obe::Utils::File::getCurrentDirectory);
    }
    void LoadFunctionSeparator(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function("separator", obe::Utils::File::separator);
    }
    void LoadFunctionGetExecutableDirectory(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function(
            "getExecutableDirectory", obe::Utils::File::getExecutableDirectory);
    }
    void LoadFunctionGetExecutablePath(sol::state_view state)
    {
        sol::table FileNamespace = state["obe"]["Utils"]["File"].get<sol::table>();
        FileNamespace.set_function(
            "getExecutablePath", obe::Utils::File::getExecutablePath);
    }
};