#pragma once

#include <fmt/format.h>
#include <string_view>

#include <Animation/Animation.hpp>
#include <Exception.hpp>

namespace obe::Animation::Exceptions
{
    class UnknownAnimationPlayMode : public Exception
    {
    public:
        UnknownAnimationPlayMode(std::string_view playMode, DebugInfo info)
            : Exception("UnknownAnimationPlayMode", info)
        {
            this->error(
                "Unable to convert the string '{}' to an AnimationPlayMode", playMode);
            this->hint("Try one of the following values : (OneTime, Loop, Force)");
        }
    };

    class UnknownAnimationGroup : public Exception
    {
    public:
        UnknownAnimationGroup(std::string_view animation, std::string_view groupName,
            const std::vector<std::string>& groups, DebugInfo info)
            : Exception("UnknownAnimationGroup", info)
        {
            this->error("Unable to retrieve AnimationGroup '{}' from Animation '{}'",
                animation, groupName);
            this->hint("Existing groups are ({})", fmt::join(groups, ", "));
        }
    };

    class AnimationTextureIndexOverflow : public Exception
    {
    public:
        AnimationTextureIndexOverflow(std::string_view animation, unsigned int index,
            unsigned int maximum, DebugInfo info)
            : Exception("AnimationTextureIndexOverflow", info)
        {
            this->error("Tried to access Texture of Animation '{}' at index {} when it "
                        "only contains {} textures",
                animation, index, maximum);
        }
    };

    class AnimationGroupTextureIndexOverflow : public Exception
    {
    public:
        AnimationGroupTextureIndexOverflow(std::string_view animationGroup,
            unsigned int index, unsigned int maximum, DebugInfo info)
            : Exception("AnimationGroupTextureIndexOverflow", info)
        {
            this->error(
                "Tried to access Texture of AnimationGroup '{}' at index {} when it "
                "only contains {} textures",
                animationGroup, index, maximum);
        }
    };

    class NoSelectedAnimationGroup : public Exception
    {
    public:
        NoSelectedAnimationGroup(std::string_view animation, DebugInfo info)
            : Exception("NoSelectedAnimationGroup", info)
        {
            this->error("The Animation '{}' does not have any AnimationGroup selected",
                animation);
        }
    };

    class UnknownAnimationCommand : public Exception
    {
    public:
        UnknownAnimationCommand(
            std::string_view animation, std::string_view command, DebugInfo info)
            : Exception("UnknownAnimationCommand", info)
        {
            this->error("Unknown command '{}' in code of Animation '{}'");
            this->hint(
                "Try one of the following valid commands (DELAY, PLAY_GROUP, CALL)");
        }
    };

    class UnknownAnimation : public Exception
    {
    public:
        UnknownAnimation(std::string_view animatorPath, std::string_view animation,
            const std::vector<std::string>& animations, DebugInfo info)
            : Exception("UnknownAnimation", info)
        {
            this->error("Animator '{}' doesn't have any Animation named '{}'");
            this->hint(
                "Try one of the following animations ({})", fmt::join(animations, ", "));
        }
    };

    class NoSelectedAnimation : public Exception
    {
    public:
        NoSelectedAnimation(std::string_view animator, DebugInfo info)
            : Exception("NoSelectedAnimation", info)
        {
            this->error(
                "The Animator '{}' does not have any Animation selected", animator);
        }
    };
}
