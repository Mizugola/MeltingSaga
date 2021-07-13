#include <Config/Templates/Animation.hpp>

namespace obe::Config::Templates
{
    vili::parser::state getAnimationTemplates()
    {
        vili::parser::state state;
        vili::node play_group = vili::object { { "command", play_group_command.data() },
            { "group", "" }, { "repeat", 1 } };
        state.push_template(play_group_command.data(), play_group);
        vili::node wait
            = vili::object { { "command", wait_command.data() }, { "time", 1.0 } };
        state.push_template(wait_command.data(), wait);
        vili::node set_animation
            = vili::object { { "command", set_animation_command.data() },
                  { "animation", "" } };
        state.push_template(set_animation_command.data(), set_animation);

        // AnimationPlayMode enum
        state.push_template("OneTime", "OneTime");
        state.push_template("Loop", "Loop");
        state.push_template("Force", "Force");
        return state;
    }
}