#pragma once

#include "pch.h"

namespace ngl
{

#define PITCH 0
#define YAW 1
#define ROLL 2

namespace mp
{

// The sub-structure pointed to by playerState_s + 0x200
struct playerState_s
{
    float origin[3];        // 0x00 - 0x0B

    uint8_t pad0[0x04];     // 0x0C - 0x0F

    float velocity[3];      // 0x10 - 0x1B

    uint8_t pad1[0xB0];     // 0x1C - 0xCB

    float viewAngles[3];    // 0xCC: Pitch, Yaw, Roll

                            // 0xD8
};

struct gentity_t
{
    uint8_t pad0[0x20];     // 0x00 - 0x1F

    float origin[3];        // 0x20 - 0x2B (reference)

    uint8_t pad1[0x134];    // 0x2C - 0x15F

    float viewAngles[3];    // 0x160 - 0x16B (reference)

    uint8_t pad2[0x94];     // 0x16C - 0x1FF

    playerState_s *ps;      // 0x200 - 0x203

    uint8_t pad3[0x68];     // 0x204 - 0x26A

    uint8_t godflag[3];     // 0x26A - 0x26D

    bool god;               // 0x26E - 0x26F

                            // 0x270
};

} // namespace mp
} // namespace ngl