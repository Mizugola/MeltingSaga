#pragma once

#include <fmt/format.h>
#include <string_view>

#include <Animation/Animation.hpp>
#include <Exception.hpp>

namespace obe::Script::Exceptions
{
    class NoSuchComponent : public Exception
    {
    public:
        NoSuchComponent(std::string_view componentType, std::string_view objectType,
            std::string_view objectId, DebugInfo info)
            : Exception("NoSuchComponent", info)
        {
            this->error("GameObject '{}' (type: '{}') has no {} component", objectId,
                objectType, componentType);
            this->hint("Try to check in the {}.obj.vili if you correctly created the "
                       "{} section",
                objectType, componentType);
        }
    };

    class ObjectDefinitionNotFound : public Exception
    {
    public:
        ObjectDefinitionNotFound(std::string_view objectType, DebugInfo info)
            : Exception("ObjectDefinitionNotFound", info)
        {
            this->error(
                "Can't find a GameObject Definition File for GameObjects of type '{}'",
                objectType);
            this->hint("Try to check if there is a file named "
                       "GameObject/{0}/{0}.obj.vili",
                objectType);
        }
    };

    class ObjectDefinitionBlockNotFound : public Exception
    {
    public:
        ObjectDefinitionBlockNotFound(std::string_view objectType, DebugInfo info)
            : Exception("ObjectDefinitionBlockNotFound", info)
        {
            this->error("Impossible to find a block named '{0}' in "
                        "Data/GameObjects/{0}/{0}.obj.vili",
                objectType);
            this->hint("Try to edit Data/GameObjects/{0}/{0}.obj.vili and add a "
                       "top-level block named {0}",
                objectType);
        }
    };

    class ScriptFileNotFound : public Exception
    {
    public:
        ScriptFileNotFound(std::string_view objectType, std::string_view objectId,
            std::string_view scriptPath, DebugInfo info)
            : Exception("ScriptFileNotFound", info)
        {
            this->error("GameObject '{}' of type '{}' tried to load script file at path "
                        "'{}' but could not find it",
                objectId, objectType, scriptPath);
        }
    };

    class WrongSourceAttributeType : public Exception
    {
    public:
        WrongSourceAttributeType(std::string_view objectType,
            std::string_view attributeName, std::string_view expectedType,
            std::string_view realType, DebugInfo info)
            : Exception("WrongSourceAttributeType", info)
        {
            this->error("GameObject '{}' tried to use attribute '{}' as a '{}' where it "
                        "should be a '{}'",
                objectType, attributeName, realType, expectedType);
        }
    };

    class InvalidScript : public Exception
    {
    public:
        InvalidScript(std::string_view path, std::string_view error, DebugInfo info)
            : Exception("InvalidScript", info)
        {
            this->error(
                "Lua Error encountered while loading script at '{}' : {}", path, error);
        }
    };

    class GameObjectScriptError : public Exception
    {
    public:
        GameObjectScriptError(std::string_view objectType, std::string_view objectId, std::string_view callback, DebugInfo info) : Exception("GameObjectScriptError", info)
        {
            this->error("Lua Error encountered while executing callback '{}' of "
                        "GameObject '{}' of type '{}'",
                callback, objectId, objectType);
        }
    };

    class LuaExecutionError : public Exception
    {
    public:
        LuaExecutionError(std::string_view errorMessage, DebugInfo info)
            : Exception("LuaExecutionError", info)
        {
            this->error(
                "Lua encountered an error while executing code :\n{}", errorMessage);
        }
    };
}