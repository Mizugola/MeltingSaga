local Route = require("Lib/Toolkit/Route");

local Commands = {};

function Commands.exit()
    _term_close();
end

return {
    Routes = {
        Route.Call(Commands.exit);
        Route.Help("Exits the Toolkit Console");
    }
};