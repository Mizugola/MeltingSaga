#include <ObEngineCore.hpp>

#include <Audio/AudioManager.hpp>
#include <Config/Config.hpp>
#include <Config/Git.hpp>
#include <Debug/Logger.hpp>
#include <Graphics/PositionTransformers.hpp>
#include <Graphics/Sprite.hpp>
#include <Input/InputButtonMonitor.hpp>
#include <System/Path.hpp>
#include <System/Plugin.hpp>

#include <soloud/soloud.h>

namespace obe
{
    void InitEngine(unsigned int surfaceWidth, unsigned int surfaceHeight)
    {
        Debug::InitLogger();

        Debug::Log->info("Using ObEngineCore (Version : {} ({}:{}))",
            Config::OBENGINE_VERSION, Config::OBENGINE_GIT_BRANCH,
            Config::OBENGINE_GIT_HASH);

        Transform::UnitVector::Init(surfaceWidth, surfaceHeight);
        Debug::Log->debug("<ObEngine> Initialising Position Transformers");
        Graphics::InitPositionTransformer();

        Debug::Log->debug("<ObEngine> Mounting paths");
        System::MountablePath::LoadMountFile();

        Debug::Log->debug("<ObEngine> Initialising NullTexture");
        Graphics::MakeNullTexture();

        Debug::Log->info("<ObEngine> Initialisation over !");
    }
}