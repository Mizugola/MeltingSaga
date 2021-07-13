#pragma once

#include <unordered_map>

#include <vili/node.hpp>

#include <Animation/AnimationGroup.hpp>
#include <System/Path.hpp>

namespace obe
{
    namespace Engine
    {
        class ResourceManager;
    }
}

namespace obe::Animation
{
    /**
     * \brief The Play Mode of an Animation.
     *        It indicates whether an Animation can be interrupted or not and
     *        what to do when the Animation is over
     * \bind{AnimationPlayMode}
     */
    enum class AnimationPlayMode
    {
        /**
         * \brief The Animation will play once and stay at last texture
         */
        OneTime,
        /**
         * \brief The Animation will play on a loop and can be interrupted any
         *        time
         */
        Loop,
        /**
         * \brief The Animation will be played once and can't be interrupted
         */
        Force
    };

    /**
     * \brief Convert a std::string containing an AnimationPlayMode in string
     *        form to an AnimationPlayMode enum value
     * \param animationPlayMode The std::string containing the AnimationPlayMode in string
     *        form
     * \return The converted value which is an AnimationPlayMode enum value
     * \throw UnknownAnimationPlayMode if the string does not represent one of the enum values
     * \bind{[mt]AnimationPlayMode.__call}
     */
    AnimationPlayMode stringToAnimationPlayMode(const std::string& animationPlayMode);
    std::ostream& operator<<(std::ostream& os, const AnimationPlayMode& m);

    /**
     * \brief The AnimationStatus indicates whether the current Animation should
     *        continue to play or call another one.
     * \bind{AnimationStatus}
     */
    enum class AnimationStatus
    {
        /**
         * \brief The Animation continues to play.
         */
        Play,
        /**
         * \brief The Animation will call another one.
         *        The name of the Animation to call is stored in
         *        m_nextAnimation. You can also get the Animation name to call by
         *        using Animation::getCalledAnimation().
         */
        Call
    };

    class Animation;

    class AnimationState
    {
    private:
        const Animation& m_parent;
        std::unordered_map<std::string, std::unique_ptr<AnimationGroup>> m_groups;
        std::size_t m_codeIndex = 0;
        bool m_feedInstructions = true;
        std::string m_currentGroupName;
        std::string m_nextAnimation;
        bool m_over = false;
        AnimationStatus m_status = AnimationStatus::Play;
        Time::TimeUnit m_clock = 0;
        Time::TimeUnit m_sleep = 0;

        void executeInstruction();
        void updateCurrentGroup();
        void setActiveAnimationGroup(const std::string& groupName);

    public:
        AnimationState(const Animation& parent);
        void load();
        /**
         * \brief Get the Animation Status
         * \return An enum value containing the AnimationStatus, it can be one
         *         of these modes :
         *         - AnimationStatus::Play
         *         - AnimationStatus::Call
         */
        [[nodiscard]] AnimationStatus getStatus() const noexcept;
        /**
         * \brief Get the name of the Animation to call when the AnimationStatus
         *        of the Animation is equal to AnimationStatus::Call
         * \return A std::string containing the name of the Animation that will be called.
         */
        [[nodiscard]] std::string getCalledAnimation() const noexcept;
        /**
         * \brief Get AnimationGroup pointer by groupName.
         *        It will throws a
         *        ObEngine.Animation.Animation.AnimationGroupNotFound if the
         *        AnimationGroup is not found.
         * \param groupName The name of the
         *        AnimationGroup to return
         * \return A pointer to the AnimationGroup
         * \throw UnknownAnimationGroup if the group does not exists
         */
        AnimationGroup& getAnimationGroup(const std::string& groupName);
        /**
         * \brief Get the name of the current AnimationGroup
         * \return A std::string containing the name of the current
         *         AnimationGroup
         */
        [[nodiscard]] std::string getCurrentAnimationGroup() const noexcept;
        /**
         * \brief Get the current Texture displayed by the Animation
         * \return A reference to the currently displayed Texture
         */
        const Graphics::Texture& getTexture();
        /**
         * \brief Return whether the Animation is over or not
         * \return true if the Animation is over, false otherwise
         */
        [[nodiscard]] bool isOver() const noexcept;
        /**
         * \brief Reset the Animation (Unselect current AnimationGroup and
         *        restart AnimationCode)
         */
        void reset() noexcept;
        /**
         * \brief Update the Animation (Updates the current AnimationGroup,
         *        executes the AnimationCode)
         */
        void update();
        const Animation& getAnimation() const;
    };

    /**
     * \brief A whole Animation that contains one or more AnimationGroup.
     * \bind{Animation}
     */
    class Animation
    {
    private:
        AnimationState m_defaultState;
        std::string m_name;
        std::unordered_map<std::string, std::unique_ptr<AnimationGroup>> m_groups;

        bool m_antiAliasing = false;
        Time::TimeUnit m_delay = 0;

        std::vector<vili::node> m_code;

        std::vector<Graphics::Texture> m_textures;

        AnimationPlayMode m_playMode = AnimationPlayMode::OneTime;

        int m_priority = 0;

        void loadMeta(const vili::node& meta);
        void loadImages(const vili::node& images, const System::Path& path,
            Engine::ResourceManager* resources);
        void loadGroups(const vili::node& groups);
        void loadCode(const vili::node& code);

        friend class AnimationState;

    public:
        Animation();
        /**
         * \todo Make Animation a serializable type instead of this "applyParameters"
         * \brief Apply global Animation parameters (Sprite offset and priority)
         * \param parameters A vili::node that contains the
         *        following optional parameters :
         *        - spriteOffsetX : x Coordinate of the Sprite Offset in the Animation in
         *                          pixels.
         *        - spriteOffsetY : y Coordinate of the Sprite Offset in the Animation in
         *                          pixels.
         *        - priority : Priority of the Animation (A higher Animation
         *                     priority can't be interrupted by an Animation with a
         *                     lower one).
         */
        void applyParameters(vili::node& parameters);
        /**
         * \brief Get the name of all contained AnimationGroup of the Animation
         * \return An array of strings with all the names of the
         *         AnimationGroup
         */
        [[nodiscard]] std::vector<std::string> getAllAnimationGroupName() const;
        /**
         * \brief Get the default delay of the Animation.
         *        The delay will be transferred to AnimationGroup children if not
         *        specified.
         * \return The default delay of the Animation in seconds.
         */
        [[nodiscard]] Time::TimeUnit getDelay() const noexcept;
        /**
         * \brief Get AnimationGroup pointer by groupName.
         *        It will throws a
         *        ObEngine.Animation.Animation.AnimationGroupNotFound if the
         *        AnimationGroup is not found.
         * \param groupName The name of the
         *        AnimationGroup to return
         * \return A pointer to the AnimationGroup
         * \throw UnknownAnimationGroup if the group does not exists
         */
        AnimationGroup& getAnimationGroup(const std::string& groupName);
        /**
         * \brief Get the Animation name
         * \return A string containing the name of the Animation
         */
        [[nodiscard]] std::string getName() const noexcept;

        /**
         * \brief Get the Animation Play Mode
         * \return An enum value containing the AnimationPlayMode, it can be one
         *         of these modes :
         *         - AnimationPlayMode::OneTime
         *         - AnimationPlayMode::Loop
         *         - AnimationPlayMode::Force
         */
        [[nodiscard]] AnimationPlayMode getPlayMode() const noexcept;
        /**
         * \brief Get the Animation Status
         * \return An enum value containing the AnimationStatus, it can be one
         *         of these modes :
         *         - AnimationStatus::Play
         *         - AnimationStatus::Call
         */
        [[nodiscard]] AnimationStatus getStatus() const noexcept;
        /**
         * \brief Get the name of the Animation to call when the AnimationStatus
         *        of the Animation is equal to AnimationStatus::Call
         * \return A std::string containing the name of the Animation that will be called.
         */
        [[nodiscard]] std::string getCalledAnimation() const noexcept;
        /**
         * \brief Get the name of the current AnimationGroup
         * \return A std::string containing the name of the current
         *         AnimationGroup
         */
        [[nodiscard]] std::string getCurrentAnimationGroup() const noexcept;
        /**
         * \brief Return the Animation priority
         * \return An int containing the priority of the Animation.
         *         Higher value is higher priority = Can't be interrupted by
         *         lower priority.
         */
        [[nodiscard]] int getPriority() const noexcept;
        /**
         * \brief Get the current Texture displayed by the Animation
         * \return A reference to the currently displayed Texture
         */
        const Graphics::Texture& getTexture();
        /**
         * \brief Get the texture used in the Animation at the specified index
         * \param index Index of the texture to return.
         * \return A reference to the Texture at the given index
         */
        const Graphics::Texture& getTextureAtIndex(int index);
        /**
         * \brief Return whether the Animation is over or not
         * \return true if the Animation is over, false otherwise
         */
        [[nodiscard]] bool isOver() const noexcept;
        /**
         * \brief Configure an Animation using the Animation configuration file
         *        (Vili file)
         * \param path System::Path to the Animation config file
         *        (.ani.vili file extension)
         * \param resources pointer to the ResourceManager
         *        that will load the textures for the Animation
         */
        void loadAnimation(
            const System::Path& path, Engine::ResourceManager* resources = nullptr);
        /**
         * \brief Reset the Animation (Unselect current AnimationGroup and
         *        restart AnimationCode)
         */
        void reset() noexcept;
        /**
         * \brief Update the Animation (Updates the current AnimationGroup,
         *        executes the AnimationCode)
         */
        void update();
        /**
         * \brief Enables or disables anti-aliasing for textures of this animation
         * \param antiAliasing should be true to enable antiAliasing, false otherwise
         */
        void setAntiAliasing(bool antiAliasing) noexcept;
        /**
         * \brief Gets the anti-aliasing status for the Animation
         */
        [[nodiscard]] bool getAntiAliasing() const noexcept;
        [[nodiscard]] AnimationState makeState() const;
    };
} // namespace obe::Animation