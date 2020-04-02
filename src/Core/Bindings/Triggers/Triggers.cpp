#include <Bindings/Triggers/Triggers.hpp>

#include <Triggers/Trigger.hpp>
#include <Triggers/TriggerGroup.hpp>
#include <Triggers/TriggerManager.hpp>

#include <sol/sol.hpp>

namespace obe::Triggers::Bindings
{
    void LoadClassTrigger(sol::state_view state)
    {
        sol::table TriggersNamespace = state["obe"]["Triggers"].get<sol::table>();
        sol::usertype<obe::Triggers::Trigger> bindTrigger
            = TriggersNamespace.new_usertype<obe::Triggers::Trigger>("Trigger",
                sol::call_constructor,
                sol::constructors<obe::Triggers::Trigger(
                                      obe::Triggers::TriggerGroup&, const std::string&),
                    obe::Triggers::Trigger(
                        obe::Triggers::TriggerGroup&, const std::string&, bool)>());
        bindTrigger["getState"] = &obe::Triggers::Trigger::getState;
        bindTrigger["getGroup"] = &obe::Triggers::Trigger::getGroup;
        bindTrigger["getName"] = &obe::Triggers::Trigger::getName;
        bindTrigger["getNamespace"] = &obe::Triggers::Trigger::getNamespace;
        bindTrigger["registerEnvironment"] = &obe::Triggers::Trigger::registerEnvironment;
        bindTrigger["unregisterEnvironment"]
            = &obe::Triggers::Trigger::unregisterEnvironment;
    }
    void LoadClassTriggerEnv(sol::state_view state)
    {
        sol::table TriggersNamespace = state["obe"]["Triggers"].get<sol::table>();
        sol::usertype<obe::Triggers::TriggerEnv> bindTriggerEnv
            = TriggersNamespace.new_usertype<obe::Triggers::TriggerEnv>("TriggerEnv",
                sol::call_constructor,
                sol::constructors<obe::Triggers::TriggerEnv(
                    sol::environment, std::string, bool*)>());
        bindTriggerEnv["environment"] = &obe::Triggers::TriggerEnv::environment;
        bindTriggerEnv["callback"] = &obe::Triggers::TriggerEnv::callback;
        bindTriggerEnv["active"] = &obe::Triggers::TriggerEnv::active;
    }
    void LoadClassTriggerGroup(sol::state_view state)
    {
        sol::table TriggersNamespace = state["obe"]["Triggers"].get<sol::table>();
        sol::usertype<obe::Triggers::TriggerGroup> bindTriggerGroup
            = TriggersNamespace.new_usertype<obe::Triggers::TriggerGroup>("TriggerGroup",
                sol::call_constructor,
                sol::constructors<obe::Triggers::TriggerGroup(
                    sol::state_view, const std::string&, const std::string&)>());
        bindTriggerGroup["setJoinable"] = &obe::Triggers::TriggerGroup::setJoinable;
        bindTriggerGroup["isJoinable"] = &obe::Triggers::TriggerGroup::isJoinable;
        bindTriggerGroup["get"] = &obe::Triggers::TriggerGroup::get;
        bindTriggerGroup["add"] = &obe::Triggers::TriggerGroup::add;
        bindTriggerGroup["remove"] = &obe::Triggers::TriggerGroup::remove;
        bindTriggerGroup["trigger"] = &obe::Triggers::TriggerGroup::trigger;
        bindTriggerGroup["pushParameter"]
            = &obe::Triggers::TriggerGroup::pushParameterFromLua;
        bindTriggerGroup["getTriggersNames"]
            = &obe::Triggers::TriggerGroup::getTriggersNames;
        bindTriggerGroup["getTriggers"] = &obe::Triggers::TriggerGroup::getTriggers;
        bindTriggerGroup["getNamespace"] = &obe::Triggers::TriggerGroup::getNamespace;
        bindTriggerGroup["getName"] = &obe::Triggers::TriggerGroup::getName;
    }
    void LoadClassTriggerManager(sol::state_view state)
    {
        sol::table TriggersNamespace = state["obe"]["Triggers"].get<sol::table>();
        sol::usertype<obe::Triggers::TriggerManager> bindTriggerManager
            = TriggersNamespace.new_usertype<obe::Triggers::TriggerManager>(
                "TriggerManager", sol::call_constructor,
                sol::constructors<obe::Triggers::TriggerManager(sol::state_view)>());
        bindTriggerManager["getTrigger"] = &obe::Triggers::TriggerManager::getTrigger;
        bindTriggerManager["getAllTriggersNameFromTriggerGroup"]
            = &obe::Triggers::TriggerManager::getAllTriggersNameFromTriggerGroup;
        bindTriggerManager["createNamespace"]
            = &obe::Triggers::TriggerManager::createNamespace;
        bindTriggerManager["createTriggerGroup"]
            = &obe::Triggers::TriggerManager::createTriggerGroup;
        bindTriggerManager["joinTriggerGroup"]
            = &obe::Triggers::TriggerManager::joinTriggerGroup;
        bindTriggerManager["getAllTriggersGroupNames"]
            = &obe::Triggers::TriggerManager::getAllTriggersGroupNames;
        bindTriggerManager["removeNamespace"]
            = &obe::Triggers::TriggerManager::removeNamespace;
        bindTriggerManager["removeTriggerGroup"]
            = &obe::Triggers::TriggerManager::removeTriggerGroup;
        bindTriggerManager["doesTriggerGroupExists"]
            = &obe::Triggers::TriggerManager::doesTriggerGroupExists;
        bindTriggerManager["update"] = &obe::Triggers::TriggerManager::update;
        bindTriggerManager["clear"] = &obe::Triggers::TriggerManager::clear;
    }
};