-- Dependancies
local String = require("Lib/StdLib/String");
local Color = require("Lib/StdLib/ConsoleColor");
local copy = require("Lib/StdLib/Copy");
local Style = require("Lib/Toolkit/Stylesheet");
local Table = require("Lib/StdLib/Table");

local currentExecution;

local fs = obe.Utils.File;

-- Load all Toolkit functions located in Lib/Toolkit/Functions
function loadToolkitFunctions()
    local toolkitFunctionsDirectory = obe.System.Path("obe://Lib/Toolkit/Functions"):find(obe.System.PathType.Directory);
    local fileList = fs.getFileList(toolkitFunctionsDirectory:path());
    local allFunctions = {};
    for _, content in pairs(fileList) do
        -- print("Loading Toolkit function :", "Lib/Toolkit/Functions/", content);
        allFunctions[String.split(content, ".")[1]] = require(
                                                          "Lib/Toolkit/Functions/" ..
                                                              String.split(
                                                                  content, ".")[1]);
    end
    return allFunctions;
end

-- Display the current input to the user
function askForCompletion(command)
    Color.print({
        {text = "> ", color = Style.CompletionPrompt.Prompt},
        {text = command, color = Style.CompletionPrompt.Text},
        {text = " ...", color = Style.CompletionPrompt.TripleDot}
    });
end

-- Checks if the value checks the Argument requirements (based on type)
function matchCommandArgumentType(content, value)
    if content.type == "Argument" then
        -- Argument type matching
        if (content.argType == "any") or
            (content.argType == "number" and tonumber(value) ~= nil) or
            (content.argType == "boolean" and
                (value == "true" or value == "false")) or
            (content.argType == "string" and
                (tonumber(value) == nil and value ~= "false" and value ~= true)) then
            return true;
        end
    end
    return false;
end

-- Returns a table with only the elements from matchList that starts like baseWord
function excludeNonMatchingWords(baseWord, matchList)
    local newMatchList = {};
    for _, content in pairs(matchList) do
        if #content >= #baseWord then
            if content:sub(0, #baseWord) == baseWord then
                table.insert(newMatchList, content);
            end
        end
    end
    return newMatchList;
end

-- We get the biggest common root of a word
-- Biggest common root of "res" in {"reset", "reselling", "reservations"} is "rese"
function getBiggestCommonRoot(baseWord, completionWords)
    local completed = (#completionWords > 0);
    local baseLetter = "";
    for index, content in pairs(completionWords) do
        if #content > #baseWord then
            if index == 1 then
                baseLetter = content:sub(#baseWord + 1, #baseWord + 1);
            elseif content:sub(#baseWord + 1, #baseWord + 1) ~= baseLetter then
                completed = false;
                break
            end
        else
            completed = false;
            break
        end
    end
    if completed == true then
        return getBiggestCommonRoot(baseWord .. baseLetter, completionWords)
    else
        return baseWord
    end
end

-- We split user input into a command table containing command name and command args
function splitCommandAndArgs(command)
    local commandName = "";
    local strArgs = "";
    local commandArgs = {};
    for k, v in pairs(String.split(command, " ")) do
        if k == 1 then
            commandName = v;
        else
            table.insert(commandArgs, v);
            if #strArgs > 0 then
                strArgs = strArgs .. " " .. v;
            else
                strArgs = v;
            end
        end
    end
    if command:sub(#command, #command) == " " then
        table.insert(commandArgs, "");
        strArgs = strArgs .. " ";
    end
    return {
        name = commandName,
        args = commandArgs,
        full = command,
        strArgs = strArgs
    };
end

-- Find if the command name does not match any other commands
function isUniqueValidCommand(command)
    local commandMatches = 0;
    for key, func in pairs(ToolkitFunctions) do
        if key:sub(0, #command.full) == command.name then
            commandMatches = commandMatches + 1;
        end
    end
    return commandMatches == 1;
end

function printHelp(arg)
    local helpParts = {{text = "? ", color = Style.Execute}};
    local command = splitCommandAndArgs(arg);
    table.insert(helpParts, {text = command.name .. " ", color = Style.Command});
    for i = 1, #command.args do
        if i == #command.args then
            table.insert(helpParts, {
                text = command.args[i] .. " ",
                color = Style.Workspace
            })
        else
            table.insert(helpParts, {
                text = command.args[i] .. " ",
                color = Style.Argument
            });
        end
    end
    table.insert(helpParts, {text = ": ", color = Style.Default});
    table.insert(helpParts, {text = getHelp(arg), color = Style.Help});
    Color.print(helpParts, 2);
end

-- Get the "Help" child in a command node
function getHelp(arg)
    if type(arg) == "table" then
        for _, child in pairs(arg) do
            if child.type == "Help" then return child.help; end
        end
    elseif type(arg) == "string" then
        local command = splitCommandAndArgs(arg);
        if ToolkitFunctions[command.name] then
            local node = reachNode(command.name, {
                id = "{root}",
                children = ToolkitFunctions[command.name].Routes
            }, command.args);
            return getHelp(node.children);
        else
            Color.print({
                {text = "Command '", color = Style.Error},
                {text = command.name, color = Style.Command},
                {text = "' not found.", color = Style.Error}
            }, 2);
        end
    end
    return "";
end

-- Checks the current command execution state
function checkCommandExecutionState(co)
    if coroutine.status(co) == "dead" then
        return false;
    else
        return true;
    end
end

-- Builds a new command execution
function buildCommandExecution(func, args)
    local autocomplete = {
        value = {func = function() print("Called autocomplete"); end}
    };
    args["autocomplete"] = autocomplete;
    local ArgMirror = require('obe://Lib/Internal/ArgMirror');
    -- Getting arguments names of the function
    local argList = ArgMirror.GetArgs(func);
    local callArgs = {};
    -- We create a table containing arguments of the function at the correct index
    for _, arg in pairs(argList) do
        if args[arg] == nil then
            error("Command does not take any parameter named '" .. arg .. "'");
        end
        table.insert(callArgs, args[arg].value);
    end
    local exec = coroutine.create(function()
        local subroutine = coroutine.create(function()
            func(ArgMirror.Unpack(callArgs));
        end);
        local ok, msg = coroutine.resume(subroutine);
        if not ok then
            print(msg);
            print(debug.traceback(subroutine))
            error(msg);
        end
        local sendBuffer;
        while checkCommandExecutionState(subroutine) do
            local wakeType, input = coroutine.yield(sendBuffer);
            sendBuffer = nil;
            if wakeType == "input" then
                local ok, msg = coroutine.resume(subroutine, input);
                if not ok then
                    error(msg);
                else
                    sendBuffer = msg;
                end
            elseif wakeType == "autocomplete" then
                sendBuffer = autocomplete.value.func(input);
            end
        end
    end);
    local ok, msg = coroutine.resume(exec);
    if not ok then error(msg); end
    return exec;
end

-- Autocomplete the argument parts of the command
function autocompleteArgs(command, query)
    local path = {};
    local currentWord = "";
    local start = "";
    -- Splitting all words to make a path
    for char in query:gmatch(".") do
        if char == " " and currentWord ~= "" then
            table.insert(path, currentWord);
            currentWord = "";
        else
            currentWord = currentWord .. char;
        end
    end
    -- We create a table containing the next arguments
    if currentWord ~= " " then start = currentWord end
    local entries = 0;
    local subpath = {};
    for _, content in pairs(path) do
        if entries ~= 0 then table.insert(subpath, content); end
        entries = entries + 1;
    end
    table.insert(subpath, start);
    -- If we need recursion to reach a sub-Node
    if entries > 0 then
        local recurseIn = nil;
        -- We iterate over children of the current command
        for id, content in pairs(command) do
            if content.type == "Node" and id == path[1] then
                -- We found a Node that matches the id
                recurseIn = content;
                break
            elseif matchCommandArgumentType(content, path[1]) then
                -- We found an Argument that matches the type
                recurseIn = content;
            end
        end
        if recurseIn then
            -- If we find a Node / Argument that matches criterias
            return autocompleteArgs(recurseIn.children,
                                    pl.List(subpath):join(" "));
        else
            -- If no Node / Argument were found
            return {};
        end
    else
        -- We are at the last argument
        local arglist = {};
        -- We iterate over all children of the current argument
        for id, arg in pairs(command) do
            -- We check the type of the current child
            if arg.type == "Node" then
                -- If we found a Node and the beginning of the current argument value matches the id of the node
                arg.name = id;
                if start ~= nil and string.sub(id, 1, string.len(start)) ==
                    start then
                    table.insert(arglist, arg);
                elseif start == nil then
                    table.insert(arglist, arg);
                end
            elseif arg.type == "Argument" then
                -- If we found an Argument child
                -- We forward-declare the function
                local autocompleteFunction;
                -- We iterate over children of the Argument
                for _, searchFunction in pairs(arg.children) do
                    -- If we find an "Autocomplete" child
                    if searchFunction.type == "Autocomplete" then
                        -- We store the function for later use
                        autocompleteFunction = searchFunction.ref;
                        break
                    end
                end
                -- If we found an autocomplete function in the Argument children
                if autocompleteFunction then
                    -- We call the autocomplete function
                    local autocompleteResult = autocompleteFunction(start);
                    -- We build a table containing all the suggestions returned by the function
                    for _, content in pairs(autocompleteResult) do
                        -- If the part of the last argument matches the beginning of the current completion
                        if start ~= nil and content:sub(1, string.len(start)) ==
                            start then
                            -- We insert it into suggestions
                            table.insert(arglist, {
                                children = {
                                    {
                                        type = "Help",
                                        help = "Argument suggestion for <" .. id ..
                                            "> parameter" .. "\n            (" ..
                                            getHelp(arg.children) .. ")"
                                    }
                                },
                                type = "Node",
                                name = content
                            });
                        elseif start == nil then
                            -- If the current argument is empty, we can insert any completion
                            table.insert(arglist, {
                                children = {
                                    {
                                        type = "Help",
                                        help = "Argument suggestion for <" .. id ..
                                            "> parameter" .. "\n            (" ..
                                            getHelp(arg.children) .. ")"
                                    }
                                },
                                type = "Node",
                                name = content
                            });
                        end
                    end
                else
                    -- If no autocompletion function were found, we display a default help
                    local defaultHelp = "Variable for argument <" .. id .. ">";
                    if getHelp(arg.children) ~= "" then
                        defaultHelp = getHelp(arg.children);
                    end
                    table.insert(arglist, {
                        children = {{type = "Help", help = defaultHelp}},
                        name = "<" .. id .. ">"
                    });
                end
            end
        end
        return arglist;
    end
end

-- That function returns what the inputbox should contain after triggering autocompletion
function autocompleteHandle(command)
    local autocompleteResult = command.full;
    -- If the command isn't empty
    if #command.full > 0 then
        -- If we find the command in available Toolkit functions
        if ToolkitFunctions[command.name] then
            -- We try to autocomplete the arguments
            local completions = autocompleteArgs(
                                    ToolkitFunctions[command.name].Routes,
                                    command.strArgs);
            -- We remove
            local nodesOnly = {};
            for _, v in pairs(completions) do
                if v.type == "Node" and v.name:sub(1, 1) ~= "<" and
                    v.name:sub(#v.name, #v.name) ~= ">" then
                    nodesOnly[v.name] = v;
                end
            end
            local completionKeys = Table.getKeys(nodesOnly);
            if Table.getSize(completionKeys) == 1 then
                -- Only one completion found, complete and add space (if not a variable)
                local onlyCompletion = completionKeys[1];
                if onlyCompletion:sub(1, 1) == "<" and
                    onlyCompletion:sub(#onlyCompletion, #onlyCompletion) == ">" then
                    -- Variable Parameter, do nothing
                else
                    -- If the last character of the autocompleteResult is a space
                    while autocompleteResult:sub(#autocompleteResult,
                                                 #autocompleteResult) ~= " " do
                        -- We remove that space
                        autocompleteResult = autocompleteResult:sub(1, -2);
                    end
                    -- We set the autocompleteResult to what should be in the inputbox
                    autocompleteResult =
                        autocompleteResult .. completionKeys[1] .. " ";
                end
            elseif Table.getSize(completionKeys) > 1 then
                -- Multiple completions found, completing with biggest common root
                -- We remove that trailing space
                while autocompleteResult:sub(#autocompleteResult,
                                             #autocompleteResult) ~= " " do
                    autocompleteResult = autocompleteResult:sub(1, -2);
                end
                -- We complete as much as we can based on the common root of available completions
                local commonRoot = getBiggestCommonRoot(
                                       command.args[#command.args],
                                       completionKeys);
                autocompleteResult = autocompleteResult .. commonRoot;
            end
        end
    end
    return autocompleteResult;
end

-- Complete the workspace function name based on current input if possible
function getMatchingToolkingFunctions(command)
    local allFunctions = {};
    for key, func in pairs(ToolkitFunctions) do
        table.insert(allFunctions, key);
    end

    -- Get all commands that match input and get the common root of all commands
    -- For example, common root of command "reset" and "restore" is "res"
    local longestPossibleCompletion = getBiggestCommonRoot(command.full,
                                                           excludeNonMatchingWords(
                                                               command.full,
                                                               allFunctions));

    -- Fill input with common root
    if #longestPossibleCompletion ~= #command.full then
        _term_write(longestPossibleCompletion:sub(#command.full + 1));
        if isUniqueValidCommand(command) then _term_write(" "); end
    end

    -- Display all possibilities
    askForCompletion(command.full);
    for key, func in pairs(ToolkitFunctions) do
        if key:sub(0, #command.full) == command.full then
            Color.print({
                {text = "> ", color = Style.Default},
                {text = key, color = Style.Command},
                {text = " : ", color = Style.Default},
                {text = getHelp(func.Routes), color = Style.Help}
            }, 2);
        end
    end
end

function autocomplete(input)
    if currentExecution == nil then
        local command = splitCommandAndArgs(input);
        _term_last();
        if ToolkitFunctions[command.name] then
            if isUniqueValidCommand(command) then
                -- Adding a space when command is valid
                if #command.args == 0 and input:sub(#input, #input) ~= " " then
                    input = input .. " ";
                    _term_write(" ");
                end
                -- We call autocompleteHandle to get what text should replace the input
                local checkCurrentInput = autocompleteHandle(command);
                -- We always show all suggestions
                local completions = autocompleteArgs(
                                        ToolkitFunctions[command.name].Routes,
                                        command.strArgs);
                if Table.getSize(completions) == 1 and completions[1].name ==
                    command.args[#command.args] then
                    input = input .. " ";
                    _term_write(" ");
                else
                    askForCompletion(input);
                    for _, completion in pairs(completions) do
                        Color.print({
                            {text = "> ", color = Style.Default},
                            {text = completion.name, color = Style.Argument},
                            {text = " : ", color = Style.Default},
                            {
                                text = getHelp(completion.children),
                                color = Style.Help
                            }
                        }, 2);
                    end
                end
                -- If a completion were found
                if input ~= checkCurrentInput then
                    _term_clear();
                    -- We modify the inputbox
                    _term_write(checkCurrentInput);
                end
            else
                -- When commands overlaps
                askForCompletion(input);
                for key, func in pairs(ToolkitFunctions) do
                    if key:sub(0, #input) == input then
                        Color.print({
                            {text = "> ", color = Style.Default},
                            {text = key, color = Style.Command},
                            {text = " : ", color = Style.Default},
                            {text = getHelp(func.Routes), color = Style.Help}
                        }, 2);
                    end
                end
            end
        else
            getMatchingToolkingFunctions(command);
        end
    else
        local status, completions = coroutine.resume(currentExecution,
                                                     "autocomplete", input);
        if completions ~= nil and type(completions) == "table" then
            local validCompletions = {};
            local completionsInfos = {};
            local defaultInfo = "Input suggestion"
            for k, v in pairs(completions) do
                local completion = v.completion or v;
                if completion:sub(1, #input) == input then
                    if type(v) == "string" then
                        table.insert(validCompletions, v);
                        table.insert(completionsInfos, defaultInfo);
                    elseif type(v) == "table" and v.completion then
                        table.insert(validCompletions, v.completion);
                        if v.info then
                            table.insert(completionsInfos, v.info);
                        else
                            table.insert(completionsInfos, defaultInfo);
                        end
                    end
                end
            end
            local autocompleteResult = "";
            for i, completion in pairs(validCompletions) do
                Color.print({
                    {text = "> ", color = Style.Default},
                    {text = completion, color = Style.Argument},
                    {text = " : ", color = Style.Default},
                    {text = completionsInfos[i], color = Style.Help}
                }, 2);
            end
            if Table.getSize(validCompletions) == 1 then
                autocompleteResult = validCompletions[1];
            elseif Table.getSize(validCompletions) > 1 then
                -- We complete as much as we can based on the common root of available completions
                local commonRoot = getBiggestCommonRoot(input, validCompletions);
                autocompleteResult = commonRoot;
            end
            _term_clear();
            _term_write(autocompleteResult);
        end
        if coroutine.status(currentExecution) == "dead" then
            currentExecution = nil;
        end
    end
end

-- Try to return a node in the command tree based on input
function reachNode(command, branch, args)
    -- Get the first part of the command arguments and store the remaining ones
    local argCount = 0;
    local subArgs = {};
    local nextJump = "";
    for _, content in pairs(args) do
        if argCount ~= 0 then
            -- Store the args after the next command argument key
            table.insert(subArgs, content);
        else
            -- Store the next command argument key
            nextJump = content;
        end
        argCount = argCount + 1;
    end
    -- If there is remaining arguments left to compute
    if nextJump ~= "" then
        local recurseIn = nil;
        -- For all arguments children in the current argument
        for id, content in pairs(branch.children) do
            -- If we find a "Node" argument that matches the next argument in the input
            if content.type == "Node" and id == nextJump then
                -- We store the Node that will be used
                recurseIn = content;
                break
            end
        end
        -- If the current argument did not match a "Node" argument before
        -- (Either because the argument did not exist or because it was an "Argument" argument)
        if recurseIn == nil then
            -- We iterate again in all arguments children in the current argument
            for id, content in pairs(branch.children) do
                -- If we find an "Argument" argument that matches the expect argType
                if matchCommandArgumentType(content, nextJump) then
                    -- We store the Argument that will be used
                    recurseIn = content;
                    break
                end
            end
        end
        -- If we found the next Node / Argument
        if recurseIn then
            -- We use recursion to reach the next Node / Argument
            return reachNode(command, recurseIn, subArgs);
        else
            -- Invalid Argument
            Color.print({
                {text = "Invalid argument ", color = Style.Error},
                {text = nextJump, color = Style.Argument}
            }, 2);
        end
    else
        -- No remaining arguments left to compute were found
        return branch;
    end
end

-- Try to return the function given a command and its arguments (Will also return identified args of the function)
function reachCommand(command, branch, args, idargs)
    -- idargs can be nil, we set it to an empty table if that's the case
    local identifiedArgs = idargs or {};
    -- Get the first part of the command arguments and store the remaining ones
    local argCount = 0;
    local subArgs = {};
    local nextJump = "";
    for _, content in pairs(args) do
        if argCount ~= 0 then
            -- Store the args after the next command argument key
            table.insert(subArgs, content);
        else
            -- Store the next command argument key
            nextJump = content;
        end
        argCount = argCount + 1;
    end
    -- If there is remaining arguments left to compute
    if nextJump ~= "" then
        local recurseIn = nil;
        -- For all arguments children in the current argument
        for id, content in pairs(branch.children) do
            -- If we find a "Node" argument that matches the next argument in the input
            if content.type == "Node" and id == nextJump then
                -- We store the Node that will be used
                recurseIn = content;
                -- We also store it as an identified argument
                identifiedArgs[id] = {
                    index = Table.getSize(identifiedArgs),
                    type = "Node"
                };
                break
            end
        end
        -- If the current argument did not match a "Node" argument before
        -- (Either because the argument did not exist or because it was an "Argument" argument)
        if recurseIn == nil then
            -- We iterate again in all arguments children in the current argument
            for id, content in pairs(branch.children) do
                -- If we find an "Argument" argument that matches the expect argType
                if matchCommandArgumentType(content, nextJump) then
                    -- We store the Argument that will be used
                    recurseIn = content;
                    -- We also store it as an identified argument
                    identifiedArgs[id] = {
                        index = Table.getSize(identifiedArgs),
                        type = "Argument",
                        argType = content.argType
                    };
                    break
                end
            end
        end
        -- If we found the next Node / Argument
        if recurseIn then
            -- We use recursion to reach the next Node / Argument
            return reachCommand(command, recurseIn, subArgs, identifiedArgs);
        else
            -- Invalid Argument
            Color.print({
                {text = "Invalid argument ", color = Style.Error},
                {text = nextJump, color = Style.Argument}
            }, 2);
        end
    else
        -- No remaining arguments left to compute were found
        local futureCall = nil;
        -- We iterate in the current branch children
        for _, content in pairs(branch.children) do
            -- If we find a "Call" Node
            if content.type == "Call" then
                -- We copy the Node for later use
                futureCall = copy(content);
                break
            end
        end
        -- If we find a callable function
        if futureCall then
            return futureCall.ref, identifiedArgs;
        else
            -- No callable function found
            Color.print({
                {text = "Nothing to call with argument '", color = Style.Error},
                {text = nextJump, color = Style.Argument},
                {text = "' in command ", color = Style.Error},
                {text = command, color = Style.Command}
            }, 2);
            return nil, nil
        end
    end
end

-- Execute user input
function evaluate(input)
    if currentExecution == nil then
        local command = splitCommandAndArgs(input);
        -- If the command is found in ToolkitFunctions
        if ToolkitFunctions[command.name] then
            -- We try to retrieve the function to call from command name
            -- We also retrieve the command excepted arguments (name, index and type (Node / Argument))
            local callFunction, identifiedArgs =
                reachCommand(command.name, {
                    id = "{root}",
                    children = ToolkitFunctions[command.name].Routes
                }, command.args);
            -- If the function to call has been found and arguments has been identified
            if callFunction and identifiedArgs then
                -- For all arguments of the command
                for key, content in pairs(identifiedArgs) do
                    if content.type == "Node" then
                        -- If the argument is a "Node", the argument value will be the name of the next Node / Argument
                        identifiedArgs[key].value =
                            Table.getKeyAtIndex(identifiedArgs,
                                                content.index + 1);
                    elseif content.type == "Argument" then
                        -- Otherwise, if the argument is an "Argument", we just fetch the value from user input
                        identifiedArgs[key].value =
                            command.args[content.index + 1];
                    end
                end
                -- We call the function and we unpack arguments
                currentExecution = buildCommandExecution(callFunction,
                                                         identifiedArgs);
                if coroutine.status(currentExecution) == "dead" then
                    currentExecution = nil;
                end
            end
        else
            -- Command not found, we display an error
            Color.print({
                {text = "Command '", color = Style.Error},
                {text = command.name, color = Style.Command},
                {text = "' not found.", color = Style.Error}
            }, 2);
        end
    else
        coroutine.resume(currentExecution, "input", input);
        if coroutine.status(currentExecution) == "dead" then
            currentExecution = nil;
        end
    end
end

-- Loading functions of the toolkit
ToolkitFunctions = loadToolkitFunctions();
-- Displaying start message
--[[Color.print({
    {text = "@", color = Style.Boot.At},
    {text = "ObEngine Toolkit ", color = Style.Boot.Text},
    {
        text = " (" .. obe.Config.OBENGINE_VERSION .. ")",
        color = Style.Boot.Version
    }
});]]
