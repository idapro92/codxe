#include "pch.h"
#include <xtl.h>
#include <stdio.h>
#include "save_load.h"
#include <game/ngl/mp/structs.h>
#include <game/ngl/mp/symbols.h>
#include <iostream>
#include <string.h>

namespace ngl
{
namespace mp
{
uint32_t liveEntityAddress;

Detour AddNCommands_Detour;
Detour SavePosition_Detour;
Detour GamePadEvent_Detour;
Detour OnPlyrSpwned_Detour;

// --- Typedefs & Original Function Pointers ---
typedef void (*tSetAngles)(uint32_t ent, float *angles);
typedef void (*tSetPosition)(uint32_t ent, float *origin);
typedef void (*tLinkEntity)(uint32_t ent);

tSetAngles Engine_SetAngles = (tSetAngles)0x825262C0;
tSetPosition Engine_SetPosition = (tSetPosition)0x825237B0;
tLinkEntity Engine_LinkEntity = (tLinkEntity)0x824F5D48;

// --- Persistent Storage ---
static float g_savedPos[3], g_savedRot[3];
static bool g_hasSaved = false;

// Global variable updated on spawn
extern uint32_t liveEntityAddress;

void DumpEntityMemory(const char *label, void *addr)
{
    if (!addr)
        return;
    unsigned char *pBase = (unsigned char *)addr;
    DbgPrint("---------------- [ %s: 0x%p ] ----------------", label, addr);

    for (int i = 0; i < 0x1000; i += 16)
    {
        uint32_t *row = (uint32_t *)(pBase + i);
        DbgPrint("+%02X: %08X %08X %08X %08X", i, row[0], row[1], row[2], row[3]);
    }
    DbgPrint("------------------------------------------------------------");
}

void LogTableStrings()
{
    // Starting static address
    uint32_t *tableEntry = (uint32_t *)0x82C2B1E0;
    const char *targetName = "SP_OP_con_main.panel";
    bool found = false;

    DbgPrint("--- Starting Table Scan at 0x%p ---\n", tableEntry);

    // Using a for loop without a constant condition to avoid C4127
    for (int i = 0; i<64; i++)
    {
        uint32_t currentAddr = tableEntry[0];
        uint32_t nextVal = tableEntry[1];

        // Sentinel check: 00000000 FFFFFFFF
        if (currentAddr == 0x00000000 && nextVal == 0xFFFFFFFF)
        {
            DbgPrint("--- End of Table Reached --- %s\n", found ? "" : "(Target NOT found)");
            break;
        }

        if (currentAddr != 0)
        {
            // String is at currentAddr + 0x1D
            const char *entryString = (const char *)(uintptr_t)(currentAddr + 0x1D);

            if (strcmp(entryString, targetName) == 0)
            {
                found = true;

                // 1. Initial calculation: Base + 0x2820
                uint32_t currentPtr = currentAddr + 0x2820;
                DbgPrint("MATCH: %s at 0x%08X\n", entryString, currentAddr);
                DbgPrint("Starting pointer chase at: 0x%08X\n", currentPtr);

                // 2. Pointer Chase Loop
                // We check if the address is valid and specifically if it starts with 0x82
                // 0x82000000 to 0x82FFFFFF
                while (currentPtr >= 0x82000000 && currentPtr <= 0x82FFFFFF)
                {
                    // Read the value stored at the current pointer
                    uint32_t nextAddr = *(uint32_t *)currentPtr;

                    // Check if the VALUE we just read also starts with 0x82
                    if ((nextAddr & 0xFF000000) == 0x82000000)
                    {
                        DbgPrint("  -> Following 0x%08X to 0x%08X\n", currentPtr, nextAddr);
                        currentPtr = nextAddr;
                    }
                    else
                    {
                        // The value doesn't start with 82, so this is our final destination
                        DbgPrint("  -> Final Destination reached: 0x%08X (Value: 0x%08X)\n", currentPtr, nextAddr);
                        DumpEntityMemory("SENSITIVITY B", (uint32_t *)currentPtr);



                        DumpEntityMemory("SENSITIVITY A", (uint32_t *)currentPtr);
                        break;
                    }
                }
            }
        }

        // Advance to next 8-byte entry (2 uint32s)
        tableEntry += 2;
    }
}

// Load logic
void LoadPosition()
{
    gentity_t *entity = (gentity_t *)liveEntityAddress;
    if (g_hasSaved && entity != 0)
    {
        entity->ps->velocity[0] = 0;
        entity->ps->velocity[1] = 0;
        entity->ps->velocity[2] = 0;
        Engine_LinkEntity(liveEntityAddress);

        Engine_SetPosition(liveEntityAddress, g_savedPos);
        Engine_SetAngles(liveEntityAddress, g_savedRot);
        Engine_LinkEntity(liveEntityAddress);
        //Sleep(1);
        CG_GameMessage("^5Position loaded");
    }
}

// --- 1. Add New Commands Hook (loadpos = Load Logic) ---
// Addr: 0x82418640
void AddNCommands_Hook()
{
    Cmd_AddCommandInternal("loadpos", LoadPosition);
    AddNCommands_Detour.GetOriginal<void>();
}

// --- 2. Melee Hook (Save Logic) ---
// Addr: 0x824ECAC0
void SavePosition_Hook(int r3, int timestamp, int r5)
{
    gentity_t *entity = (gentity_t *)liveEntityAddress;
    if (entity->ps)
    {
        playerState_s *ps = entity->ps;
        static int lastPressTime = 0;
        int delta = timestamp - lastPressTime;
        if (lastPressTime != 0 && delta > 50 && delta < 450)
        {
            memcpy(g_savedPos, ps->origin, sizeof(float) * 3);
            memcpy(g_savedRot, ps->viewAngles, sizeof(float) * 3);
            g_hasSaved = true;
            CG_GameMessage("^5Position saved");
        }
        lastPressTime = timestamp;
    }
    SavePosition_Detour.GetOriginal<tSavePos>()(r3, timestamp, r5);
}

// --- 3. Gamepad Event Hook (Catch All) ---
// Addr: 0x824F45A8
void GamePadEvent_Hook(int clientNum, int button, int down)
{
    int controllerIndex = clientNum;
    switch (controllerIndex)
    {
        case DPAD_UP:
        {
            //gentity_t *entity = (gentity_t *)liveEntityAddress;
            //DumpEntityMemory("PLAYERSTATE", entity->ps);
            CG_GameMessage("^2DPAD UP");
            break;
        }
        case DPAD_RIGHT:
        {
            CG_GameMessage("^3DPAD RIGHT");
            break;
        }
        case DPAD_DOWN:
        {
            CG_GameMessage("^4DPAD DOWN");
            break;
        }
        case DPAD_LEFT:
        {
            CG_GameMessage("^6DPAD LEFT");
            break;
        }
        default:
            break;
    }
    GamePadEvent_Detour.GetOriginal<tGamePad>()(clientNum, button, down);
}

// --- 4. Player Spawned Hook (Diagnostics) ---
// Addr: 0x82202EE8
void OnPlyrSpwned_Hook(uint32_t r3, uint32_t r4)
{
    uint32_t globalPointerAddr = EntityArray_Ptr; // 0x82B8C6FC
    uint32_t entityArrayBase = *(uint32_t *)globalPointerAddr; // 0x82744910
    uint32_t predictedPtrLoc = entityArrayBase + 0x4; // 0x82744914
    liveEntityAddress = *(uint32_t *)predictedPtrLoc; // [ENTITY ADDRESS from 0x82744914]
    gentity_t *entity = (gentity_t *)liveEntityAddress; // [ENTITY OBJ]
    entity->god = true;
    //Cbuf_AddText("god\n");
    CG_GameMessage("^5Welcome to CoDJumper");
    //Cbuf_AddText("set bg_fallDamageMaxHeight 9999999\n");
    //Cbuf_AddText("set bg_fallDamageMinHeight 9999999\n");
    
    OnPlyrSpwned_Detour.GetOriginal<tOnSpawn>()(r3, r4);
}
    // --- Lifecycle ---
void save_load::install_detours()
{
    // 1. Add new commands (loadpos = Load Logic)
    AddNCommands_Detour =
        Detour(reinterpret_cast<void *>(AddNCommands_Addr), reinterpret_cast<void *>(AddNCommands_Hook));
    AddNCommands_Detour.Install();

    // 2. Melee (Save Logic)
    SavePosition_Detour =
        Detour(reinterpret_cast<void *>(SavePosition_Addr), reinterpret_cast<void *>(SavePosition_Hook));
    SavePosition_Detour.Install();

    // 3. GamePad (Input Catch)
    GamePadEvent_Detour =
        Detour(reinterpret_cast<void *>(GamePadEvent_Addr), reinterpret_cast<void *>(GamePadEvent_Hook));
    GamePadEvent_Detour.Install();

    // 4. OnPlyrSpwned (Diagnostics/Sync) 
    OnPlyrSpwned_Detour =
        Detour(reinterpret_cast<void *>(OnPlyrSpwned_Addr), reinterpret_cast<void *>(OnPlyrSpwned_Hook));
    OnPlyrSpwned_Detour.Install();
}

save_load::save_load()
{
    install_detours();
}

save_load::~save_load()
{
    SavePosition_Detour.Remove();
    GamePadEvent_Detour.Remove();
    OnPlyrSpwned_Detour.Remove();
    AddNCommands_Detour.Remove();
}

} // namespace mp
} // namespace ngl