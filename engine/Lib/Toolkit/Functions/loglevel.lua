local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");

local Commands = {};

local function get_log_levels()
    return {
        "0 (trace)",
        "1 (debug)",
        "2 (info)",
        "3 (warn)",
        "4 (err)",
        "5 (critical)",
        "6 (off)"
    };
end

function Commands.loglevel(level)
    local log_levels = get_log_levels();
    local level_num = tonumber(level);
    if level_num ~= nil and level_num >= 0 and level_num <= 6 then
        obe.Debug.setLevel(level_num);
        Color.print({
            { text = "Log level set to ", color = Style.Default},
            { text = log_levels[level_num+1], color = Style.Argument},
        }, 1);
    else
        Color.print({
            {text = "Invalid argument ", color = Style.Error},
            {text = level, color = Style.Argument}
        }, 2);
    end
end 

return {
    Routes = {
        Route.Help("Change log level of ÖbEngine");
        level = Route.Arg {
            Route.Help("Log level");
            Route.Call(Commands.loglevel);
            Route.Autocomplete(get_log_levels);
        };
    }
};