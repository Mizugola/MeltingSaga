#include <Config/Templates/Mount.hpp>
#include <Utils/FileUtils.hpp>

namespace obe::Config::Templates
{
    vili::parser::state getMountTemplates()
    {
        vili::parser::state state;

        vili::node workspace
            = vili::object { { "type", "Workspace" }, { "path", "" }, { "priority", 2 } };
        state.push_template("Workspace", workspace);

        vili::node package
            = vili::object { { "type", "Package" }, { "path", "" }, { "priority", 1 } };
        state.push_template("Package", package);

        vili::node path
            = vili::object { { "type", "Path" }, { "path", "" }, { "priority", 0 } };
        state.push_template("Path", path);

        vili::node executable = vili::object { { "type", "Path" },
            { "path", Utils::File::getExecutableDirectory() }, { "priority", 0 } };
        state.push_template("Executable", executable);

        return state;
    }
}