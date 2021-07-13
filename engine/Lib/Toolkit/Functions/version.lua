local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");

local Commands = {};

function Commands.version()
    Color.print({
        { text = "ÖbEngine is in version ", color = Style.Default},
        { text = obe.version, color = Style.Workspace},
    }, 1);
end

function Commands.major()
    local major_version = string.split(string.sub(obe.version, 2),".")[1];
    Color.print({
        { text = "ÖbEngine's major version is ", color = Style.Default},
        { text = major_version, color = Style.Workspace},
    }, 1);
end

function Commands.minor()
    local minor_version = string.split(string.sub(obe.version, 2),".")[2];
    Color.print({
        { text = "ÖbEngine's minor version is ", color = Style.Default},
        { text = minor_version, color = Style.Workspace},
    }, 1);
end

function Commands.patch()
    local patch_version = string.split(string.sub(obe.version, 2),".")[3];
    Color.print({
        { text = "ÖbEngine's patch version is ", color = Style.Default},
        { text = patch_version, color = Style.Workspace},
    }, 1);
end

function Commands.commit()
    local commit_version = obe.commit;
    Color.print({
        { text = "ÖbEngine's commit version is ", color = Style.Default},
        { text = commit_version, color = Style.Workspace},
    }, 1);
end

function Commands.branch()
    local branch_version = obe.branch;
    Color.print({
        { text = "ÖbEngine's branch version is ", color = Style.Default},
        { text = branch_version, color = Style.Workspace},
    }, 1);
end

return {
    Routes = {
        Route.Help("Get ÖbEngine's version");
        Route.Call(Commands.version);
        major = Route.Node {
            Route.Help("Get ÖbEngine's major version");
            Route.Call(Commands.major);
        },
        minor = Route.Node {
            Route.Help("Get ÖbEngine's minor version");
            Route.Call(Commands.minor);
        },
        patch = Route.Node {
            Route.Help("Get ÖbEngine's patch version");
            Route.Call(Commands.patch);
        },
        commit = Route.Node {
            Route.Help("Get ÖbEngine's commit version");
            Route.Call(Commands.commit);
        },
        branch = Route.Node {
            Route.Help("Get ÖbEngine's branch version");
            Route.Call(Commands.branch);
        }
    }
};