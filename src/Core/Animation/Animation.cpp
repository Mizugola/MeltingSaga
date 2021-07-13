#include <vili/node.hpp>
#include <vili/parser.hpp>
#include <vili/types.hpp>
#include <vld8/validator.hpp>

#include <Animation/Animation.hpp>
#include <Animation/Exceptions.hpp>
#include <Config/Templates/Animation.hpp>
#include <Config/Validators.hpp>
#include <Debug/Logger.hpp>
#include <Engine/ResourceManager.hpp>
#include <Utils/StringUtils.hpp>

namespace obe::Animation
{
    AnimationPlayMode stringToAnimationPlayMode(const std::string& animationPlayMode)
    {
        if (animationPlayMode == "OneTime")
            return AnimationPlayMode::OneTime;
        if (animationPlayMode == "Loop")
            return AnimationPlayMode::Loop;
        if (animationPlayMode == "Force")
            return AnimationPlayMode::Force;
        throw Exceptions::UnknownAnimationPlayMode(animationPlayMode, EXC_INFO);
    }

    std::ostream& operator<<(std::ostream& os, const AnimationPlayMode& m)
    {
        os << "AnimationPlayMode::";
        switch (m)
        {
        case AnimationPlayMode::Force:
            os << "Force";
            break;
        case AnimationPlayMode::Loop:
            os << "Loop";
            break;
        case AnimationPlayMode::OneTime:
            os << "OneTime";
            break;
        }
        return os;
    }

    AnimationState::AnimationState(const Animation& parent)
        : m_parent(parent)
    {
    }

    void AnimationState::load()
    {
        for (const auto& [groupName, group] : m_parent.m_groups)
        {
            m_groups.emplace(groupName, std::make_unique<AnimationGroup>(*group.get()));
        }
    }

    std::string AnimationState::getCalledAnimation() const noexcept
    {
        return m_nextAnimation;
    }

    std::string Animation::getCalledAnimation() const noexcept
    {
        return m_defaultState.getCalledAnimation();
    }

    std::string Animation::getName() const noexcept
    {
        return m_name;
    }

    Time::TimeUnit Animation::getDelay() const noexcept
    {
        return m_delay;
    }

    AnimationGroup& AnimationState::getAnimationGroup(const std::string& groupName)
    {
        if (const auto group = m_groups.find(groupName); group != m_groups.end())
            return *group->second;
        throw Exceptions::UnknownAnimationGroup(
            m_parent.getName(), groupName, m_parent.getAllAnimationGroupName(), EXC_INFO);
    }

    AnimationGroup& Animation::getAnimationGroup(const std::string& groupName)
    {
        return m_defaultState.getAnimationGroup(groupName);
    }

    std::string AnimationState::getCurrentAnimationGroup() const noexcept
    {
        return m_currentGroupName;
    }

    std::string Animation::getCurrentAnimationGroup() const noexcept
    {
        return m_defaultState.getCurrentAnimationGroup();
    }

    std::vector<std::string> Animation::getAllAnimationGroupName() const
    {
        std::vector<std::string> animationGroupKeys;
        animationGroupKeys.reserve(m_groups.size());
        std::transform(m_groups.cbegin(), m_groups.cend(), animationGroupKeys.begin(),
            [](const auto& pair) { return pair.first; });
        return animationGroupKeys;
    }

    AnimationPlayMode Animation::getPlayMode() const noexcept
    {
        return m_playMode;
    }

    AnimationStatus AnimationState::getStatus() const noexcept
    {
        return m_status;
    }

    AnimationStatus Animation::getStatus() const noexcept
    {
        return m_defaultState.getStatus();
    }

    bool AnimationState::isOver() const noexcept
    {
        return m_over;
    }

    bool Animation::isOver() const noexcept
    {
        return m_defaultState.isOver();
    }

    void Animation::loadAnimation(
        const System::Path& path, Engine::ResourceManager* resources)
    {
        Debug::Log->debug("<Animation> Loading Animation at {0}", path.toString());
        const std::string animationConfigFile
            = path.add(path.last() + ".ani.vili").find();
        vili::node animationConfig = vili::parser::from_file(
            animationConfigFile, Config::Templates::getAnimationTemplates());

        try
        {
            vili::validator::validate_tree(
                Config::Validators::AnimationValidator(), animationConfig);

            // Meta
            this->loadMeta(animationConfig.at("Meta"));
            Debug::Log->trace("  <Animation> Loading Meta block");

            // Images
            this->loadImages(animationConfig.at("Images"), path, resources);
            Debug::Log->trace("  <Animation> Loading Images block");

            // Groups
            Debug::Log->trace("  <Animation> Loading Groups block");
            this->loadGroups(animationConfig.at("Groups"));

            // Animation Code
            Debug::Log->trace("  <Animation> Loading Animation block");
            this->loadCode(animationConfig.at("Animation"));

            m_defaultState.load();
        }
        catch (const Exception& e)
        {
            throw Exceptions::InvalidAnimationFile(animationConfigFile, EXC_INFO).nest(e);
        }
    }

    void AnimationState::executeInstruction()
    {
        const vili::node& currentCommand = m_parent.m_code[m_codeIndex];
        Debug::Log->trace("<Animation> Executing instruction {} / {} : {}", m_codeIndex,
            m_parent.m_code.size() - 1, currentCommand.dump());
        if (currentCommand.at("command").as_string() == Config::Templates::wait_command)
        {
            m_feedInstructions = true;
            m_sleep = currentCommand.at("time");
        }
        else if (currentCommand.at("command").as_string()
            == Config::Templates::play_group_command)
        {
            if (!m_currentGroupName.empty())
                m_groups[m_currentGroupName]->reset();
            m_feedInstructions = false;
            m_currentGroupName = currentCommand.at("group");
            m_groups[m_currentGroupName]->setLoops(currentCommand.at("repeat"));
        }
        else if (currentCommand.at("command").as_string()
            == Config::Templates::set_animation_command)
        {
            m_feedInstructions = false;
            m_status = AnimationStatus::Call;
            m_nextAnimation = currentCommand.at("animation");
        }
        else
        {
            throw Exceptions::UnknownAnimationCommand(
                m_parent.m_name, currentCommand.at("command"), EXC_INFO);
        }
        m_codeIndex++;
        if (m_feedInstructions && m_codeIndex > m_parent.m_code.size() - 1
            && m_parent.m_playMode != AnimationPlayMode::OneTime)
        {
            this->reset();
        }
    }

    void AnimationState::updateCurrentGroup()
    {
        Debug::Log->trace(
            "    <Animation> Updating AnimationGroup '{}'", m_currentGroupName);
        m_groups[m_currentGroupName]->next();
        if (m_groups[m_currentGroupName]->isOver())
        {
            Debug::Log->trace(
                "        <Animation> AnimationGroup '{}' is over", m_currentGroupName);
            if (m_codeIndex < m_parent.m_code.size())
            {
                Debug::Log->trace("    <Animation> Restarting code execution");
                m_feedInstructions = true;
                m_groups[m_currentGroupName]->reset();
            }
            else
            {
                Debug::Log->trace(
                    "    <Animation> Animation '{}' has no more code to execute");
                if (m_parent.m_playMode == AnimationPlayMode::OneTime)
                {
                    Debug::Log->trace("    <Animation> Animation '{}' will stay on "
                                      "the last texture");
                    m_groups[m_currentGroupName]->previous(true);
                    m_over = true;
                }
                else
                {
                    Debug::Log->trace(
                        "    <Animation> Animation '{}' will reset code execution");
                    m_feedInstructions = true;
                    m_groups[m_currentGroupName]->reset();
                    m_codeIndex = 0;
                }
            }
        }
    }

    void AnimationState::setActiveAnimationGroup(const std::string& groupName)
    {
        if (m_groups.find(groupName) != m_groups.end())
        {
            m_currentGroupName = groupName;
        }
        throw Exceptions::UnknownAnimationGroup(
            m_parent.m_name, groupName, m_parent.getAllAnimationGroupName(), EXC_INFO);
    }

    void Animation::loadMeta(const vili::node& meta)
    {
        m_name = meta.at("name");

        Debug::Log->trace("    <Animation> Animation name = '{}'", m_name);
        if (meta.contains("clock"))
        {
            m_delay = meta.at("clock");
            Debug::Log->trace("    <Animation> Animation clock = {}", m_delay);
        }
        if (meta.contains("mode"))
        {
            m_playMode = stringToAnimationPlayMode(meta.at("mode"));
            Debug::Log->trace("    <Animation> Animation play-mode = '{}'", m_playMode);
        }
    }

    void Animation::loadImages(const vili::node& images, const System::Path& path,
        Engine::ResourceManager* resources)
    {
        const vili::node& imageList = images.at("images");
        std::string model;
        if (images.contains("model"))
        {
            model = images.at("model");
            Debug::Log->trace(
                "    <Animation> Using following template to load images : {}", model);
        }
        for (auto& image : imageList)
        {
            std::string textureName;
            if (image.is<vili::integer>() && !model.empty())
            {
                textureName = Utils::String::replace(
                    model, "%s", std::to_string(image.as<vili::integer>()));
                Debug::Log->trace("    <Animation> Loading image '{}' (name determined "
                                  "with template[int])",
                    textureName);
            }
            else if (image.is<vili::string>() && !model.empty())
            {
                textureName = Utils::String::replace(model, "%s", image);
                Debug::Log->trace("    <Animation> Loading image '{}' (name determined "
                                  "with template[str])",
                    textureName);
            }
            else if (image.is<vili::string>())
            {
                textureName = image;
                Debug::Log->trace("    <Animation> Loading image '{}'", textureName);
            }

            std::string pathToTexture = path.add(textureName).toString();
            Debug::Log->trace(
                "    <Animation> Found Texture Path at '{}'", pathToTexture);
            if (resources)
            {
                Debug::Log->trace(
                    "    <Animation> Loading Texture {0} (using ResourceManager)",
                    textureName);
                m_textures.emplace_back(
                    resources->getTexture(path.add(textureName), m_antiAliasing));
            }
            else
            {
                Debug::Log->trace("    <Animation> Loading Texture {0}", textureName);
                Graphics::Texture newTexture;
                newTexture.loadFromFile(path.add(textureName).find());
                // TODO: Add a way to configure anti-aliasing for textures without ResourceManager
                m_textures.push_back(std::move(newTexture));
            }
        }
    }

    void Animation::loadGroups(const vili::node& groups)
    {
        for (auto [groupName, group] : groups.items())
        {
            Debug::Log->trace("    <Animation> Loading AnimationGroup '{}'", groupName);
            m_groups.emplace(groupName, std::make_unique<AnimationGroup>(groupName));
            for (vili::node& currentTexture : group.at("content"))
            {
                Debug::Log->trace("      <Animation> Pushing Texture {} into group",
                    currentTexture.as<vili::integer>());
                m_groups[groupName]->pushTexture(
                    m_textures[currentTexture.as<vili::integer>()]);
            }

            if (group.contains("clock"))
            {
                vili::number delay = group.at("clock");
                Debug::Log->trace("      <Animation> Setting group delay to {}", delay);
                m_groups[groupName]->setDelay(delay);
            }
            else
            {
                Debug::Log->trace(
                    "      <Animation> No delay specified, using parent delay : {}",
                    m_delay);
                m_groups[groupName]->setDelay(m_delay);
            }
        }
    }

    void Animation::loadCode(const vili::node& code)
    {
        for (const vili::node& command : code.at("code"))
        {
            Debug::Log->trace("    <Animation> Parsing Animation command '{}'", command);
            m_code.push_back(command);
        }
    }

    Animation::Animation()
        : m_defaultState(*this)
    {
    }

    void Animation::applyParameters(vili::node& parameters)
    {
        // TODO: Re-implement texture offset in a better way
        if (parameters.contains("priority"))
            m_priority = parameters.at("priority");
    }

    void AnimationState::update()
    {
        if (!m_over)
        {
            const Time::TimeUnit delay = (m_sleep) ? m_sleep : m_parent.m_delay;
            Debug::Log->trace("<Animation> Delay is {} seconds", delay);
            if (Time::epoch() - m_clock > delay)
            {
                m_clock = Time::epoch();
                m_sleep = 0;
                Debug::Log->trace(
                    "<Animation> Updating Animation '{0}'", m_parent.m_name);

                if (m_feedInstructions)
                {
                    this->executeInstruction();
                }
                if (!m_currentGroupName.empty())
                {
                    this->updateCurrentGroup();
                }
            }
        }
    }

    const Animation& AnimationState::getAnimation() const
    {
        return m_parent;
    }

    void Animation::setAntiAliasing(bool antiAliasing) noexcept
    {
        m_antiAliasing = antiAliasing;
    }

    bool Animation::getAntiAliasing() const noexcept
    {
        return m_antiAliasing;
    }

    AnimationState Animation::makeState() const
    {
        return AnimationState(*this);
    }

    void AnimationState::reset() noexcept
    {
        Debug::Log->trace("<Animation> Resetting Animation '{}'", m_parent.m_name);
        for (auto it = m_groups.cbegin(); it != m_groups.cend(); ++it)
            it->second->reset();
        m_status = AnimationStatus::Play;
        m_codeIndex = 0;
        m_feedInstructions = true;
        m_over = false;
    }

    void Animation::reset() noexcept
    {
        m_defaultState.reset();
    }

    void Animation::update()
    {
        m_defaultState.update();
    }

    const Graphics::Texture& Animation::getTextureAtIndex(int index)
    {
        if (index < m_textures.size())
            return m_textures[index];
        throw Exceptions::AnimationTextureIndexOverflow(
            m_name, index, m_textures.size(), EXC_INFO);
    }

    const Graphics::Texture& AnimationState::getTexture()
    {
        if (!m_currentGroupName.empty())
            return m_groups[m_currentGroupName]->getTexture();
        throw Exceptions::NoSelectedAnimationGroup(m_parent.m_name, EXC_INFO);
    }

    const Graphics::Texture& Animation::getTexture()
    {
        return m_defaultState.getTexture();
    }

    int Animation::getPriority() const noexcept
    {
        return m_priority;
    }
} // namespace obe::Animation