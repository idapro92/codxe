#include "pch.h"
#include "main.h"
#include "components/project_velocity.h"
#include "components/save_load.h"

namespace ngl
{
namespace mp
{

void setupPatches()
{
    // Unlimited game time
    *(unsigned int *)0x82057D4F = 0x00000000;
    // No respawn time
    *(unsigned int *)0x821D4F64 = 0x38600000;
    // Remove grass (Xenia fix)
    *(unsigned int *)0x823D6450 = 0x4e800020;

    // Unlimited sprint
    *(unsigned int *)0x8245AfE4 = 0x60000000;
    *(unsigned int *)0x8245B364 = 0x60000000;

    // Unlimited ammo
    *(unsigned int *)0x824533A4 = 0x60000000;

    // Fall damage fix
    *(unsigned int *)0x82454534 = 0x48000214;

    // Horizontal sensitivity
    *(unsigned int *)0x824CC4C4 = 0x3920001A;   //Default value
    *(unsigned int *)0x82C2B674 = 0x0000001A;   //Update
    
    // Vertical sensitivity
    *(unsigned int *)0x824CC4CC = 0x39000032;   //Default value
    *(unsigned int *)0x82C2B678 = 0x00000032;   //Update
    // Sound volume
    *(unsigned int *)0x824CC530 = 0x39600018;

    // Default game mode (mp_gametype)
    *(unsigned int *)0x8205ABF4 = 0x74646D00;

    //System link patch
    *(unsigned int *)0x823CC52C = 0x4800000C;
    //*(unsigned int *)0x823C09A8 = 0x38A00000;
    //*(unsigned int *)0x821F1B50 = 0x3D608205;
    //*(unsigned int *)0x821F1B58 = 0x388BABE0;

    

    //*(unsigned int *)0x823C2598 = 0x3D608205;
    //*(unsigned int *)0x823C259C = 0x388BABE0;
}

NGL_MP_Plugin::NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin initialized\n");

    setupPatches();
    
    auto save = new save_load();
    
    auto bouncing = new project_velocity();
    

    // 2. Register them
    RegisterModule(bouncing);
    
    RegisterModule(save);
    

    // 3. Explicitly init in order
    // This ensures Audio is UP before Save/Load tries to play sounds
    //AudioSystem::Init();
    
}

NGL_MP_Plugin::~NGL_MP_Plugin()
{
    DbgPrint("NGL MP Plugin shutting down\n");
}

} // namespace mp
} // namespace ngl