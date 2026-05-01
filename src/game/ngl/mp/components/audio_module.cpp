#include "pch.h"
#include "audio_module.h"
#include "audio_manager.h"

namespace ngl
{
namespace mp
{

audio_module::audio_module()
{
}
audio_module::~audio_module()
{
    AudioSystem::Shutdown();
}

void audio_module::play_sound(const char *alias)
{
    // This will trigger AudioSystem::Init() internally if it's the first call
    AudioSystem::Play(alias);
}

} // namespace mp
} // namespace ngl