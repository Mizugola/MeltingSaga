local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");
local SampleProjectTemplate = require("Lib/Toolkit/Templates/SampleProject");

local Commands = {};

local fs = obe.Utils.File;

local function is_in_engine_directory()
    return fs.fileExists("Workspace/Workspaces.vili");
end

local function contains(t, item)
    for _, v in pairs(t) do
        if v == item then
            return true;
        end
    end
    return false;
end

local function get_workspace_list()
    local workspace_definition_filepath = obe.System.Path("obe://Workspace/Workspaces.vili"):find(obe.System.PathType.File);
    local parser = vili.parser.from_file(workspace_definition_filepath:path(),
                                         vili.parser.state());
    local workspaces = vili.to_lua(parser);
    local all_workspaces = {};
    for workspace_name, _ in pairs(workspaces) do
        table.insert(all_workspaces, workspace_name);
    end
    return all_workspaces;
end

local function get_non_indexed_workspaces()
    local lookup_path = fs.getCurrentDirectory();
    if is_in_engine_directory() then
        lookup_path = fs.getCurrentDirectory() .. "/Workspace";
    end
    print("Lookup path", lookup_path);
    local all_directories = fs.getDirectoryList(lookup_path);
    local all_indexed_workspaces = get_workspace_list();

    local return_workspaces = {};
    for _, key in pairs(all_directories) do
        if not contains(all_indexed_workspaces, key) then
            table.insert(return_workspaces, key);
        end
    end
    return return_workspaces;
end

local function write_to_file(path, content)
    print("Opening file", path)
    local file_handle = io.open(path, "w");
    file_handle:write(content);
    file_handle:close();
end

Commands.mount = function(workspace_name)
    local workspace_definition_filepath = obe.System.Path(
        "obe://Workspace/Workspaces.vili"
    ):find(obe.System.PathType.File);
    local parser = vili.parser.from_file(workspace_definition_filepath:path(),
                                         vili.parser.state());
    local workspaces = vili.to_lua(parser);
    Color.print({
        {text = "Mounting Workspace '", color = Style.Execute},
        {text = workspace_name, color = Style.Workspace},
        {text = "' ...", color = Style.Execute}
    }, 1);
    if workspaces[workspace_name] then
        fs.copy(workspaces[workspace_name].path .. "/Mount.vili", "Mount.vili");
        Color.print({
            {text = "Workspace '", color = Style.Success},
            {text = workspace_name, color = Style.Workspace},
            {text = "' has  been successfully mounted !", color = Style.Success}
        }, 2);
        obe.MountPaths();
    else
        Color.print({
            {text = "Workspace '", color = Style.Error},
            {text = workspace_name, color = Style.Workspace},
            {text = "' does not exists", color = Style.Error}
        }, 2);
    end
end

Commands.create = function(workspaceName)
    local workspaceDefinitionFilepath = obe.System.Path("obe://Workspace/Workspaces.vili"):find(obe.System.PathType.File);
    local parser = vili.parser.from_file(workspaceDefinitionFilepath:path(),
                                         vili.parser.state());
    local workspaces = vili.to_lua(parser);
    Color.print({
        {text = "Creating Workspace '", color = Style.Execute},
        {text = workspaceName, color = Style.Workspace},
        {text = "' ...", color = Style.Execute}
    }, 1);
    if workspaces[workspaceName] then
        Color.print({
            {text = "Workspace '", color = Style.Error},
            {text = workspaceName, color = Style.Workspace},
            {text = "' already exists", color = Style.Error}
        }, 2);

        return;
    end
    local path = workspaceName;

    if is_in_engine_directory() then
        path = "Workspace/" .. workspaceName;
    end
    fs.createDirectory(path);
    fs.createDirectory(path .. "/Data");
    fs.createDirectory(path .. "/Data/GameObjects");
    fs.createDirectory(path .. "/Scenes");
    fs.createDirectory(path .. "/Sprites");

    write_to_file(path .. "/Data/GameObjects/SampleObject.lua", SampleProjectTemplate.HELLO_WORLD_GO_SCRIPT);
    write_to_file(path .. "/Data/GameObjects/SampleObject.obj.vili", SampleProjectTemplate.HELLO_WORLD_GO_DEF);
    write_to_file(path .. "/Scenes/sample.map.vili", SampleProjectTemplate.HELLO_WORLD_SCENE);
    write_to_file(path .. "/boot.lua", SampleProjectTemplate.HELLO_WORLD_BOOT);

    workspaces[workspaceName] = {
        path = fs.getCurrentDirectory() .. "/" .. path
    };

    workspaces = vili.from_lua(workspaces);
    local workspaceFile = io.open(workspaceDefinitionFilepath:path(), "w");
    workspaceFile:write(vili.writer.dump(workspaces, vili.writer.dump_options()));
    workspaceFile:close();

    local defaultMountFile = io.open(path .. "/Mount.vili", "w")

    local defaultMount = {
        Mount = {
            [workspaceName] = {
                type = "Workspace",
                path = path,
                priority = 1
            }
        }
    };
    defaultMount = vili.from_lua(defaultMount);

    defaultMountFile:write(vili.writer.dump(defaultMount, vili.writer.dump_options()));
    defaultMountFile:close()

    Color.print({
        {text = "Workspace '", color = Style.Success},
        {text = workspaceName, color = Style.Workspace},
        {text = "' has  been successfully created !", color = Style.Success}
    }, 2);
end

function Commands.list()
    local allWorkspaces = get_workspace_list();
    Color.print(
        {{text = "All Registered Workspaces : ", color = Style.Execute}}, 1);
    for _, key in pairs(allWorkspaces) do
        Color.print({
            {text = "- Workspace : ", color = Style.Default},
            {text = key, color = Style.Workspace}
        }, 2);
    end
end

function Commands.index(workspace_name)
    local non_indexed_workspaces = get_non_indexed_workspaces();
    if contains(non_indexed_workspaces, workspace_name) then
        local workspace_definition_filepath = obe.System.Path(
            "obe://Workspace/Workspaces.vili"
        ):find(obe.System.PathType.File);
        local parser = vili.parser.from_file(workspace_definition_filepath:path(),
                                         vili.parser.state());
        local workspaces = vili.to_lua(parser);

        local path = fs.getCurrentDirectory();
        if is_in_engine_directory() then
            path = path .. "/Workspace/" .. workspace_name;
        else
            path = path .. "/" .. workspace_name;
        end

        workspaces[workspace_name] = {
            path = path
        };
        
        workspaces = vili.from_lua(workspaces);
        write_to_file(workspace_definition_filepath:path(), vili.writer.dump(workspaces, vili.writer.dump_options()))

        Color.print({
            {text = "Workspace '", color = Style.Success},
            {text = workspace_name, color = Style.Workspace},
            {text = "' has  been successfully indexed !", color = Style.Success}
        }, 2);
    else
        local existing_workspaces = get_workspace_list();
        local directory_not_found = true;
        for _, existing_workspace in pairs(existing_workspaces) do
            if existing_workspace == workspace_name then
                directory_not_found = false;
                break;
            end
        end
        if directory_not_found then
            Color.print({
                {text = "Directory '", color = Style.Error},
                {text = workspace_name, color = Style.Workspace},
                {text = "' not found", color = Style.Error}
            }, 2);
        else
            Color.print({
                {text = "Workspace '", color = Style.Error},
                {text = workspace_name, color = Style.Workspace},
                {text = "' is already indexed", color = Style.Error}
            }, 2);
        end
    end
end

return {
    Routes = {
        Route.Help("Commands to work with Workspaces"),
        create = Route.Node {
            Route.Help("Creates a new Workspace"),
            workspaceName = Route.Arg {
                Route.Help("Name of the new Workspace to create"),
                Route.Call(Commands.create)
            }
        },
        mount = Route.Node {
            Route.Help("Mounts a Workspace"),
            workspaceName = Route.Arg {
                Route.Call(Commands.mount),
                Route.Help("Name of the Workspace you want to mount"),
                Route.Autocomplete(get_workspace_list)
            }
        },
        index = Route.Node {
            Route.Help("Indexes an existing Workspace"),
            workspace_name = Route.Arg {
                Route.Call(Commands.index),
                Route.Help("Name of the Workspace you want to index"),
                Route.Autocomplete(get_non_indexed_workspaces)
            }
        },
        list = Route.Node {
            Route.Help("Lists all existing Workspaces"),
            Route.Call(Commands.list)
        }
    }
};
