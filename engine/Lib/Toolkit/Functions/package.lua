local Color = require("Lib/StdLib/ConsoleColor");
local Route = require("Lib/Toolkit/Route");
local Style = require("Lib/Toolkit/Stylesheet");

local Package = obe.Package;

local Commands = {};

function getPackageList()
    local parser = Vili.ViliParser.new();
    parser:parseFile("Package/Packages.vili");
    local allPackages = {};
    for _, key in pairs(parser:root():getAll(Vili.NodeType.ComplexNode)) do 
        table.insert(allPackages, key:getId());
    end
    return allPackages;
end

function getUninstalledPackageList()
    local fileList = obe.Filesystem.getFileList("Package/");
    local opaqueFiles = {};
    local extension = ".opaque";
    for _, file in pairs(fileList) do
        if file:sub(-extension:len()) == extension then
            table.insert(opaqueFiles, file:sub(1, #file - #extension));
        end
    end
    return opaqueFiles;
end

function Commands.install(packageName)
    Color.print({
            { text = "Installing Package '", color = Style.Execute},
            { text = packageName, color = Style.Package},
            { text = "' ...", color = Style.Execute},
        }, 1)
    if Package.Install(packageName) then
        Color.print({
            { text = "Package '", color = Style.Success},
            { text = packageName, color = Style.Package},
            { text = "' has been successfully installed", color = Style.Success}
        }, 2);
        local parser = Vili.ViliParser.new();
        parser:parseFile("Package/Opaque.vili");
        local tPackageName = parser:root():at("Meta"):getDataNode("name"):getString();
        if parser:hasFlag("Mount") then
            obe.Filesystem.copy(Package.GetPackageLocation(tPackageName) .. "/Mount.vili", "Mount.vili");
        end
    else
        Color.print({
            { text = "Package '", color = Style.Error},
            { text = packageName, color = Style.Package},
            { text = "' has not been installed (Already installed ?)", color = Style.Error}
        }, 2);
    end
end

function Commands.mount(packageName)
    Color.print({
        { text = "Mounting Package '", color = Style.Execute},
        { text = packageName, color = Style.Package},
        { text = "' ...", color = Style.Execute},
    }, 1)
    if Package.PackageExists(packageName) then
        obe.Filesystem.copy(Package.GetPackageLocation(packageName) .. "/Mount.vili", "Mount.vili");
        Color.print({
            { text = "Package '", color = Style.Success},
            { text = packageName, color = Style.Package},
            { text = "' has been successfully mounted", color = Style.Success}
        }, 2);
        obe.MountPaths();
    else
        Color.print({
            { text = "Package '", color = Style.Error},
            { text = packageName, color = Style.Package},
            { text = "' does not exists", color = Style.Error}
        }, 2);
    end
end

function Commands.list()
    local allPackages = getPackageList();
    Color.print({
        { text = "All Registered Packages : ", color = Style.Execute}
    }, 1);
    for _, key in pairs(allPackages) do 
        Color.print({
            { text = "- Package : ", color = Style.Default},
            { text = key, color = Style.Package}
        }, 2);
    end
end

return {
    Routes = {
        Route.Help("Commands to work with Packages");
        install = Route.Node {
            Route.Help("Installs a Package");
            packageName = Route.Arg {
                Route.Help("Name of the .opaque package file you want to install");
                Route.Call(Commands.install);
                Route.Autocomplete(getUninstalledPackageList);
            };
        };
        mount = Route.Node {
            Route.Help("Mount a Package");
            packageName = Route.Arg {
                Route.Help("Name of the package you want to mount");
                Route.Call(Commands.mount);
            };
            Route.Autocomplete(getPackageList)
        };
        list = Route.Node {
            Route.Help("Lists all Packages");
            Route.Call(Commands.list);
        }
    }
};