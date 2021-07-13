return {
    Node = function(children)
        return {
            children = children,
            type = "Node",
        };
    end,

    Arg = function(children)
        return {
            children = children,
            type = "Argument",
            argType = "any"
        };
    end,

    NumberArg = function(children)
        return {
            children = children,
            type = "Argument",
            argType = "number"
        };
    end,

    StringArg = function(children)
        return {
            children = children,
            type = "Argument",
            argType = "string"
        };
    end,

    BooleanArg = function(children)
        return {
            children = children,
            type = "Argument",
            argType = "boolean"
        };
    end,

    Call = function(func)
        if type(func) ~= "function" then
            error("argument to Route.Call must be a function (got " .. tostring(func) .. ")");
        end
        return {
            type = "Call",
            ref = func
        };
    end,

    Autocomplete = function(func)
        return {
            type = "Autocomplete",
            ref = func
        }
    end,

    Help = function(helpString)
        return {
            type = "Help",
            help = helpString
        }
    end,
}