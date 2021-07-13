# Changelog

## 0.4.2

### Minor changes

- Removed useless method `setTarget(target)` from Canvas (replaced with `render(target)`)
- Added default "Hello, world!" project in ÖbEngine, no more crashes when running ÖbEngine for the first time !
- Better exception handling when loading an Animation file
- Added Vili templates for `AnimationPlayMode` enum

### Bugfixes

- Fixed crash when loading a GameObject without Script component

## 0.4.1

### Bugfixes

- Fixed crash when a texture is not found
- Added traceback when a crash happen in Scene's onLoad callback
- Fixed corrupted return values of `getFillColor()` and `getOutlineColor` for Canvas Shapes
- Fixed bug when there was not enough suggestions during an exception
- Fixed invalid value forwarding in vili's node::push
- Fixed bug that ignored the Sprite position defined in .obj.vili
- Fixed bug that swapped GameObject id and type in `NoSuchComponent` exception

## 0.4.0

### Notes

A bit more than two months after 0.3.0 release, here comes the version 0.4 !

There was plans to remove Vili support and move everything to Lua (configuration files, scenes...) but I decided to backpedal and think more about it. In the end, I kept Vili and upgraded it to version 2.0. Here are a few reasons of why I decided to do so :
- Lua can't keep track of the order of insertion
- Exporting Lua data can be tricky
- Unsafe configuration files
- I like Vili :)

You will probably notice a few subtle changes in the Vili files around the engine, the Vili codebase has been massively simplified and this is why some old features are not here anymore (ranges, links, template functions) but these sacrifices are not in vain !

One of this update biggest change is the introduction of a whole brand new exception system, debugging should now be easier. Exceptions now have more details and can even give a few hints on how to fix the problem !
This task took me a lot of time to complete as I had to rewrite all existing exceptions and create new one for the whole ÖbEngine codebase. Exceptions are also handled differently than before, in v0.3.0, when an exception occured in `Event.Game.Update` for example, the GameObject would spam the log with the same exception everytime it happened, now if an exception is raised, the whole engine closes gracefully so you can properly inspect what is going on.

The Canvas also have its share of changes with three new elements : Polygons, Bezier curves and textures. Textures can be applied to all existing shapes of the Canvas.

For this update I was not alone, I got help from [@PierrickLP](https://github.com/PierrickLP) [@Darnagof](https://github.com/Darnagof) and [@Mari0nV](https://github.com/Mari0nV), thanks again !

### Major changes

- Upgraded Vili to version 2.0, new syntax for all the engine configuration / scene files [#231](https://github.com/ObEngine/ObEngine/issues/231) [#232](https://github.com/ObEngine/ObEngine/issues/232) [#263](https://github.com/ObEngine/ObEngine/issues/263) [#276](https://github.com/ObEngine/ObEngine/issues/276) [#277](https://github.com/ObEngine/ObEngine/issues/277) [#278](https://github.com/ObEngine/ObEngine/issues/278)
- Greatly improved exception system with hints and proper traceback [#156](https://github.com/ObEngine/ObEngine/issues/156)
- Standardized time unit, every time value in the engine is now as seconds (double precision float) [#255](https://github.com/ObEngine/ObEngine/issues/255)
- Added Bezier curves, Polygons and Sprites (as Rect textures) support to the Canvas [#161](https://github.com/ObEngine/ObEngine/issues/161)
- Engine now properly closes when an exception is encountered [#226](https://github.com/ObEngine/ObEngine/issues/226)

### Minor changes

- Possibility to import lua files from mounted paths with `require` function (thanks to [@Mari0nV](https://github.com/Mari0nV)) [#275](https://github.com/ObEngine/ObEngine/issues/275)
- All Sprite attributes can now be modified from configuration file (thanks to [@PierrickLP](https://github.com/PierrickLP)) [#270](https://github.com/ObEngine/ObEngine/issues/270) [#273](https://github.com/ObEngine/ObEngine/issues/273)
- Better collision checks results (thanks to [@Darnagof](https://github.com/Darnagof)) [#271](https://github.com/ObEngine/ObEngine/issues/271)
- It is now possible to remove a child from a SceneNode (suggestion of [@PierrickLP](https://github.com/PierrickLP)) [#256](https://github.com/ObEngine/ObEngine/issues/256)
- More flexibility when editing Canvas.Text attributes [#197](https://github.com/ObEngine/ObEngine/issues/197)
- Improvements to the the Color class [#249](https://github.com/ObEngine/ObEngine/issues/249) [#261](https://github.com/ObEngine/ObEngine/issues/261)
- Logger now available from Lua [#280](https://github.com/ObEngine/ObEngine/issues/280)
- Removed a lot of dead code / useless resources [#279](https://github.com/ObEngine/ObEngine/issues/279)
- Optimized and fixed Input system [#229](https://github.com/ObEngine/ObEngine/issues/229)
- Optimized CallbackScheduler system [#247](https://github.com/ObEngine/ObEngine/issues/247)
- Merged window.cfg.vili into config.cfg.vili (Window section)
- Bumped version of fmt, spdlog and catch
- ÖbEngine now uses Lua 5.4 (with sol3:latest)

## 0.3.0

### Notes

v0.3.0 is a big release, and for multiple reasons.

First of all, a lot of cleaning was involved, mostly on the API (see minor changes), some elements were renamed or moved so expect quite a lot of breaking changes on your v0.2.0 projects (easy to fix with search-replace though).

The second biggest change is the deletion of the ObEngineDev executable : all the UI part was poorly architectured and too much rework would have been required to port it to v0.3.0. I decided to remove it entirely.

This means that the Map Editor and the Toolkit are not part of ÖbEngine anymore (until v0.6.0 probably), to compensate for this, I decided to included an executable for each GitHub release (separate download) which is the build result of this repository "[ÖbEngine patched editor](https://github.com/ObEngine/ObEnginePatchedEditor)".

This repository basically contains the code from the latest v0.2.0 commit of ÖbEngine with some changes to make it v0.3.0 compatible.
This means that you can use the ÖbEngine patched editor to edit Scenes (mostly to edit Sprites and Colliders), export the Scene and use it in the v0.3.0 ÖbEnginePlayer.

One of the other biggest changes is that ÖbEngine now uses [sol3](https://github.com/ThePhD/sol3) instead of [kaguya](https://github.com/satoren/kaguya) for binding Lua.
Kaguya was pretty awesome but unfortunately isn't updated anymore by its author, ÖbEngine will be updated to use Lua 5.4 as soon as it is available and I wanted a binding library able to use the new features. Also sol3 should bring some performance improvements.
With this library change comes another huge change : All bindings are now automatically generated.
Writing Lua bindings is a long, boring and tedious task, you have to handle all corner-cases.
I decided to extend the possibilities of my existing ongoing project [Obidog](https://github.com/ObEngine/Obidog) to not only generate Lua documentation (coming soon !) but also to generate all the Lua bindings and that only from the XML output of Doxygen.

Vili is now deprecated and its Lua bindings are not part of ÖbEngine anymore, this means, don't use Vili for serializing your data, prefer using Lua as v0.4.0 will entierely remove Vili to replace it with Lua. SFML bindings also disappeared and were replaced by similar ÖbEngine classes.

The v0.3.0 version also brings more freedom when it comes to folder architecture of your project. Scenes aren't expected to be in `Data/Maps` anymore and Sprites aren't expected to be located in `Sprites/Levelsprites`. GameObjects will probably receive the same treatment for a future update.

This update brings many more changes, please read the Major / Minor changes section to learn more about it !

### Major changes
- Renamed / moved some important components of the engine
	- `LevelSprite` is now called `Sprite`
	- Event table is now named `Event` instead of `Global` (`Event.Cursor.Move` for example)
	- All `Registrable` classes (`Window`, `Cursor`, `Script`, `ResourceManager`, `InputManager`, `TriggerManager`) are now part of the central `Engine` class and must be used as `Engine.Component` in Lua
- [kaguya](https://github.com/satoren/kaguya) (Lua binding library) has been replaced by [sol3](https://github.com/ThePhD/sol3)
- ObEngineDev has been removed (see notes)
- ObEngine now includes a CLI [debugger](https://github.com/slembcke/debugger.lua)
- Vili and SFML do not have Lua bindings anymore
- New Color class with more functionalities (Hex, HSV, color names support)
- More freedom for project folder architecture, Scenes / Sprites can be placed anywhere in the project folder
- Added a CallbackScheduler inside TriggerManager (allow things like JS `setTimeout` and `setInterval`)

### Minor changes
- `Canvas.Text` can't have multiple colors anymore (this feature will be back with a better API)
- `AudioManager` now accepts a LoadPolicy argument to choose whether you want to Load, Cache or Stream your audio
- Removed all display methods from `PolygonalCollider`
- Removed all child / parent methods from `PolygonalCollider` (superseded by SceneNodes)
- Merged `TimeCheck` and `Chronometer` into a single Chronometer class
- `InputButtonMonitor` instances are now handled by InputManager
- Most [PenLight](https://github.com/Tieske/Penlight) libs are no longer included by default
- [TGUI](https://github.com/texus/TGUI) is no longer a dependency
- No more missing attributes / methods lua bindings
- Most [SFML](https://github.com/SFML/SFML) classes have been replaced by ÖbEngine classes internally
- Huge API clean-up
	- No more raw-pointers unless necessary
	- Flagging [[nodiscard]] when needed
	- Some types have been modified for better / easier usage
	- Removed useless files / attributes / methods / functions
	- No more global values / singletons
	- Renamed functions / methods for simplification
	- Lua binding lib types replaced with STL types (LuaFunction to std::function for example) to allow C++ usage
	- Updated documentation where needed
	- Fixed typos in names / documentation
- Added string litterals for easy filesystem usage
- Added time units (milliseconds, secondes, hours etc..)

### Bugfixes
- No more crashes when quitting the engine, the memory is correctly deallocated
- Non-fullscreen apps are now correctly scaled


## 0.2.0

### Notes

v0.2.0 most major change is the new Audio backend.

Leaving SFML-Audio was not very hard as the only piece of code that used the audio were the Lua bindings.
This means, ÖbEngine doesn't rely on OpenAL anymore and doesn't have any LGPL contraints anymore.

Every ÖbEngine dependency can now be statically linked without any restriction.

This release also brings easier setup on Windows as you won't have to copy dll by hand anymore, this process is now automated.

The UI also supports high-dpi screens now, no more tiny window in the center of your screen where you have to use a magnifier to see on what button you are clicking.

Overall, this is a small release before the next bigger release : 0.3.0.

### Major changes

- :hammer: Automated build / release jobs with GitHub Actions
- :sound: New Audio API / Backend (from SFML-Audio to Soloud)
- :book: DLL are now copied on the execution directory on Windows
- :tv: UI now support high-dpi screens

### Minor changes

- :mute: Vili files are now silently parsed by default
- :no_entry_sign: Animation, LevelSprite and ResourceManager now support anti-aliasing parameter
- :octocat: Git variables (OBENGINE_COMMIT, OBENGINE_VERSION) are now configured as a file instead of a define
- :gear: Config is now part of Config namespace instead of System namespace
- :checkered_flag: InputCombinationElement now uses a FlagSet instead of a std::vector
- :rocket: ObEngineCore now has a InitEngine function which initialize most of the engine components
- :computer: Simplified GameObject components code
- :left_right_arrow: Added UnitVector::distance method
- :heavy_multiplication_x: GameObject cache is now cleared when the MapEditor is closed, no need to restart engine anymore

### Bugfixes
- :black_nib: Retrieving an id from a Canvas Element in Lua now works
- :bug: Fixed ProtectedIdentifiable bindings
- :bug: Fixed UI anti-aliasing
- :red_circle: Added error message when [Component]Script::source/sources had the wrong Vili type
- :black_nib: Fixed wrong alignment of Text in Canvas

## v0.1.0

### Notes

First semver release of ÖbEngine
