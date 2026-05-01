#pragma once
#include "pch.h"

namespace ngl
{
namespace mp
{

class audio_module : public Module
{
  public:
    audio_module();
    virtual ~audio_module();

    // Static method: can be called as audio_module::play_sound("alias")
    static void play_sound(const char *alias);
};

} // namespace mp
} // namespace ngl