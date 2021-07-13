local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");
local TM = require("Lib/Toolkit/Utils");

local Quests = {
    MyFirstQuest = {
        description = "A Quest for beginners !",
        quest = function(autocomplete)
            Color.print("Hello adventurer ! This is your very first quest right ?");
            Color.print("This is an adventure inside a game engine terminal...");
            Color.print("I hope you don't mind !");
            Color.print("Soo, let's imagine you have three rooms in front of you");
            Color.print("The first one is blue, the other one is red, the last one is green");
            Color.print("Which one do you choose ?");
            autocomplete.func = TM.completions {
                {"blue", "The nice cyan-looking room"},
                {"red", "The edgy crimson room"},
                {"green", "Green, because you like nature and all that stuff" }
            };
            ::askcolor::
            local choice = TM.input();
            if choice == "red" then
                Color.print("Well, now you are in hell, that's what you wanted isn't it ?");
            elseif choice == "blue" then
                Color.print("You enter the blue room ! It has a nice blue color, unexpected...");
            elseif choice == "green" then
                Color.print("You enter the green room ! It smells like dead mooses for some reason");
            else
                Color.print("Even a middle schooler knows its colors ! Try again !");
                goto askcolor;
            end
            Color.print("You exit the room and feel a bit exhausted");
            Color.print("Well, that's about it for this quest, now it's your turn to create new quests");
        end
    }
};
local function get_quests_names()
    local quests_names = {};
    for id, _ in pairs(Quests) do
        table.insert(quests_names, id);
    end
    return quests_names;
end

local Commands = {};

function Commands.create(quest_name)
end

function Commands.list()
    for id, quest in pairs(Quests) do
        Color.print({
            {text = "Quest (", color = Style.Default},
            {text = id, color = Style.Argument},
            {text = ") : ", color = Style.Default},
            {text = quest.description, color = Style.Help}
        }, 2);
    end
end

function Commands.start(autocomplete, questName)
    if Quests[questName] then
        Quests[questName].quest(autocomplete);
    else
        Color.print({{text = "This quest doesn't exist !", color = Style.Error}});
    end
end

return {
    Routes = {
        Route.Help("Commands to work with Workloads");
        create = Route.Node {
            Route.Help("Creates a new Quest");
            quest_name = Route.StringArg {
                Route.Help("Name of the new Quest to create");
                Route.Call(Commands.create);
            };
        };
        list = Route.Node {
            Route.Help("Lists all available quests");
            Route.Call(Commands.list);
        };
        start = Route.Node {
            Route.Help("Indexes an existing Workload");
            questName = Route.Arg {
                Route.Call(Commands.start);
                Route.Help("Name of the Quest you want to start");
                Route.Autocomplete(get_quests_names);
            };
        }
    }
};