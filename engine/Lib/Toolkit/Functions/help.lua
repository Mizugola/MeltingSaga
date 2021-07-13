local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");

local Commands = {};

function Commands.help()
    Color.print({
        { text = "Available commands : ", color = Style.Execute}
    }, 1);
    for k, v in pairs(ToolkitFunctions) do
        local command_help = "";
        for khelp, vhelp in pairs(v.Routes) do
            if vhelp.type == "Help" then
                command_help = vhelp.help;
                break;
            end
        end
        Color.print({
            { text = "> ", color = Style.Default},
            { text = k, color = Style.Command},
            { text = " : ", color = Style.Default},
            { text = command_help, color = Style.Help}
        }, 2);
    end
end

return {
    Routes = {
        Route.Call(Commands.help);
        Route.Help("Well, it.. helps you ?");
    }
};