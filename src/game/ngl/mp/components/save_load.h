#pragma once

#include "pch.h"

namespace ngl
{
namespace mp
{

// The class is the only thing the rest of the app needs to see
class save_load : public Module
{
  public:
    save_load();
    ~save_load();

  private:
    void install_detours();
};

} // namespace mp
} // namespace ngl