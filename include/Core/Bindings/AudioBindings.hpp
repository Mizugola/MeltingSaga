#pragma once

namespace kaguya
{
    class State;
}

namespace obe::Bindings::AudioBindings
{
    void LoadAudioManager(kaguya::State* lua);
}