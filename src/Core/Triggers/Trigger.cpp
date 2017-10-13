#include <Script/GameObject.hpp>
#include <Triggers/Trigger.hpp>
#include <Triggers/TriggerGroup.hpp>
#include <Utils/TypeUtils.hpp>

namespace obe
{
    namespace Triggers
    {
        std::string Trigger::getTriggerLuaTableName() const
        {
            return this->getNamespace() + "__" + this->getGroup() + "__" + m_name;
        }


        Trigger::Trigger(TriggerGroup* parent, const std::string& name, bool startState)
        {
            m_name = name;
            m_parent = parent;
            m_enabled = startState;
            m_fullName = this->getNamespace() + "." + this->getGroup() + "." + m_name;
        }

        bool Trigger::getState() const
        {
            return m_enabled;
        }

        std::string Trigger::getGroup() const
        {
            return m_parent->getName();
        }

        std::string Trigger::getName() const
        {
            return m_name;
        }

        std::string Trigger::getNamespace() const
        {
            return m_parent->getNamespace();
        }

        void Trigger::registerEnvironment(unsigned int envIndex, const std::string& callbackName)
        {
            Debug::Log->trace("<Trigger> Registering Lua Environment {0} in Trigger {1} associated to callback", envIndex, m_name, callbackName);
            m_registeredEnvs.emplace_back(envIndex, callbackName);
            Script::ScriptEngine["__ENVIRONMENTS"][envIndex]["LuaCore"]["FTCP"][this->getNamespace() + "__" + this->getGroup() + "__" + m_name] = kaguya::NewTable();
        }

        void Trigger::unregisterEnvironment(unsigned envIndex)
        {
            Debug::Log->trace("<Trigger> Unregistering Lua Environment {0} from Trigger {1}", envIndex, m_name);
            m_registeredEnvs.erase(std::remove_if(m_registeredEnvs.begin(), m_registeredEnvs.end(), 
                [&envIndex](const std::pair<unsigned int, std::string>& env) { return env.first == envIndex; }
            ), m_registeredEnvs.end());
        }

        void Trigger::execute() const
        {
            Debug::Log->trace("<Trigger> Executing Trigger {0}", m_name);
            std::vector<unsigned int> EnabledEnvs = Script::GameObject::AllEnvs;
            unsigned int envAmount = m_registeredEnvs.size();
            for (unsigned int envIndex = 0; envIndex < envAmount; envIndex++)
            {
                std::pair<unsigned int, std::string> rEnv = m_registeredEnvs[envIndex];
                if (Utils::Vector::isInList(rEnv.first, EnabledEnvs))
                {
                    bool envEnabled = Script::ScriptEngine["__ENVIRONMENTS"][rEnv.first]["__ENV_ENABLED"];
                    bool envDebug = Script::ScriptEngine["__ENVIRONMENTS"][rEnv.first]["__ENV_DEBUG"];
                    bool objEnabled = Script::ScriptEngine["__ENVIRONMENTS"][rEnv.first]["__OBJECT_INIT"];
                    if ((envEnabled || envDebug) && objEnabled)
                    {
                        Debug::Log->trace("<Trigger> Calling Trigger Callback {0} on Lua Environment {1} from Trigger {2}", rEnv.second, rEnv.first, m_name);
                        Script::ScriptEngine["ExecuteStringOnEnv"]
                        ("LuaCore.FuncInjector(" + rEnv.second + ", \"" + this->getTriggerLuaTableName() + "\")", rEnv.first);
                    }
                    //Script::ScriptEngine["__ENVIRONMENTS"][rEnv.first]["LuaCore"]["FuncInjector"](rEnv.second, this->getTriggerLuaTableName());
                    Script::ScriptEngine["__ENVIRONMENTS"]
                        [rEnv.first]["LuaCore"]["FTCP"]
                        [this->getTriggerLuaTableName()] = kaguya::NewTable();
                }
            }
        }

        void Trigger::pushParameterFromLua(const std::string& name, kaguya::LuaRef parameter)
        {
            Debug::Log->trace("<Trigger> Pushing parameter {0} to Trigger {1} (From Lua)", name, m_fullName);
            for (auto& rEnv : m_registeredEnvs)
            {
                Debug::Log->trace("<Trigger> Pushing parameter {0} to Lua Environment {1} from Trigger {2} (From Lua)", name, rEnv.first, m_fullName);
                Script::ScriptEngine["__ENVIRONMENTS"][rEnv.first]["LuaCore"]["FTCP"][this->getTriggerLuaTableName()][name] = parameter;
            }
        }
    }
}
