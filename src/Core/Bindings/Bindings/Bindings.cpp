#include <Bindings/Bindings/Bindings.hpp>

#include <Engine/Engine.hpp>

#include <sol/sol.hpp>

namespace obe::Bindings::Bindings
{
void LoadFunctionIndexAllBindings(sol::state_view state)
{
sol::table BindingsNamespace = state["obe"]["Bindings"].get<sol::table>();
BindingsNamespace.set_function("IndexAllBindings", obe::Bindings::IndexAllBindings);}
};