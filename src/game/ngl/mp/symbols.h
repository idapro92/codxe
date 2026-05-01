#pragma once
#include <cstdint>

#include "structs.h"

namespace ngl
{
namespace mp
{
static auto Cbuf_AddText = reinterpret_cast<void (*)(const char *text)>(0x82445008);
static auto Cmd_AddCommandInternal = reinterpret_cast<void (*)(const char *cmdName, void *callback)>(0x8244FA60);
static auto CG_GameMessage = reinterpret_cast<void (*)(const char *message)>(0x8241B808);

// Hook Addresses
static const uint32_t SavePosition_Addr = 0x824ECAC0;               // MeleePressed
static const uint32_t AddNCommands_Addr = 0x82418640;               // AddNewCommands
static const uint32_t GamePadEvent_Addr = 0x824F45A8;               // CL_GamepadButtonEvent
static const uint32_t OnPlyrSpwned_Addr = 0x82202EE8;               // G_Player_Spawn

// Function Call Addresses
static const uint32_t Engine_SetAngles_Addr = 0x825262C0;
static const uint32_t Engine_SetPosition_Addr = 0x825237B0;
static const uint32_t Engine_LinkEntity_Addr = 0x824F5D48;

// Memory Pointer Addresses
static const uint32_t EntityArray_Ptr = 0x82B8C6FC;

// Hook Typedefs
typedef void (*tSavePos)(int clientNum, int timestamp, int r5);     // Melee_t
typedef void (*tGamePad)(int clientNum, int button, int down);      // Gamepad
typedef void (*tOnSpawn)(uint32_t r3, uint32_t r4);                 // OnPlayerSpawned

// Engine Caller Typedefs
typedef void (*tSetAngles)(uint32_t ent, float *angles);
typedef void (*tSetPosition)(uint32_t ent, float *origin);
typedef void (*tLinkEntity)(uint32_t ent);

// Other possible useful addresses
//static const uint32_t CG_RespawnEventAddr = 0x824FA438;
//static const uint32_t SV_LevelInitEventAddr = 0x82559E04;         // onLevelLoad()
//static const uint32_t onPlayerJoinAddr = 0x82205B00;              // onPlayerJoin() - game loaded (no team selected)
//static const uint32_t onPlayerEnterAddr = 0x82205D50;             // onPlayerEnter() - (team selected)
//static const uint32_t G_Player_Spawn = 0x82202EE8;                // onPlayerSpawn() (r3, r4)

enum ControllerInput
{
    DPAD_UP = 154,
    DPAD_DOWN = 155,
    DPAD_LEFT = 156,
    DPAD_RIGHT = 157,

    RB = 217,
    LB = 218,

    R3 = 219
};

} // namespace mp
} // namespace ngl
