#pragma once

namespace sol
{
    class state_view;
};
namespace obe::Utils::File::Bindings
{
    void LoadFunctionGetDirectoryList(sol::state_view state);
    void LoadFunctionGetFileList(sol::state_view state);
    void LoadFunctionFileExists(sol::state_view state);
    void LoadFunctionDirectoryExists(sol::state_view state);
    void LoadFunctionCreateDirectory(sol::state_view state);
    void LoadFunctionCreateFile(sol::state_view state);
    void LoadFunctionCopy(sol::state_view state);
    void LoadFunctionDeleteFile(sol::state_view state);
    void LoadFunctionDeleteDirectory(sol::state_view state);
    void LoadFunctionGetCurrentDirectory(sol::state_view state);
    void LoadFunctionSeparator(sol::state_view state);
    void LoadFunctionGetExecutableDirectory(sol::state_view state);
    void LoadFunctionGetExecutablePath(sol::state_view state);
};