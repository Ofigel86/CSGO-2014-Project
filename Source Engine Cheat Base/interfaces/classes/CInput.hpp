#pragma once
#include "../../math/Vector.hpp"
#include "../../math/QAngle.hpp"
#include "../../utilities/checksum/checksum_crc.hpp"
#include <cstring>

#define IN_ATTACK  (1 << 0)
#define IN_JUMP   (1 << 1)
#define IN_DUCK   (1 << 2)
#define IN_FORWARD  (1 << 3)
#define IN_BACK   (1 << 4)
#define IN_USE   (1 << 5)
#define IN_CANCEL  (1 << 6)
#define IN_LEFT   (1 << 7)
#define IN_RIGHT  (1 << 8)
#define IN_MOVELEFT  (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2  (1 << 11)
#define IN_RUN   (1 << 12)
#define IN_RELOAD  (1 << 13)
#define IN_ALT1   (1 << 14)
#define IN_ALT2   (1 << 15)
#define IN_SCORE  (1 << 16)
#define IN_SPEED  (1 << 17)
#define IN_WALK   (1 << 18)
#define IN_ZOOM   (1 << 19)
#define IN_WEAPON1 (1 << 20)
#define IN_WEAPON2 (1 << 21)
#define IN_BULLRUSH (1 << 22)

class CUserCmd
{
public:
    CUserCmd()
    {
        memset(this, 0, sizeof(*this));
    };

    virtual ~CUserCmd() {};

    CRC32_t GetChecksum(void) const
    {
        CRC32_t crc;
        CRC32_Init(&crc);

        CRC32_ProcessBuffer(&crc, &m_command_number, sizeof(m_command_number));
        CRC32_ProcessBuffer(&crc, &m_tickcount, sizeof(m_tickcount));
        CRC32_ProcessBuffer(&crc, &m_viewangles, sizeof(m_viewangles));
        CRC32_ProcessBuffer(&crc, &m_aimdirection, sizeof(m_aimdirection));
        CRC32_ProcessBuffer(&crc, &m_forwardmove, sizeof(m_forwardmove));
        CRC32_ProcessBuffer(&crc, &m_sidemove, sizeof(m_sidemove));
        CRC32_ProcessBuffer(&crc, &m_upmove, sizeof(m_upmove));
        CRC32_ProcessBuffer(&crc, &m_buttons, sizeof(m_buttons));
        CRC32_ProcessBuffer(&crc, &m_impulse, sizeof(m_impulse));
        CRC32_ProcessBuffer(&crc, &m_weaponselect, sizeof(m_weaponselect));
        CRC32_ProcessBuffer(&crc, &m_weaponsubtype, sizeof(m_weaponsubtype));
        CRC32_ProcessBuffer(&crc, &m_random_seed, sizeof(m_random_seed));
        CRC32_ProcessBuffer(&crc, &m_mousedx, sizeof(m_mousedx));
        CRC32_ProcessBuffer(&crc, &m_mousedy, sizeof(m_mousedy));

        CRC32_Final(&crc);

        return crc;
    }

    int     m_command_number;   // 0x04
    int     m_tickcount;        // 0x08
    QAngle  m_viewangles;       // 0x0C
    Vector  m_aimdirection;     // 0x18
    float   m_forwardmove;      // 0x24
    float   m_sidemove;         // 0x28
    float   m_upmove;           // 0x2C
    int     m_buttons;          // 0x30
    char    m_impulse;          // 0x34
    int     m_weaponselect;     // 0x38
    int     m_weaponsubtype;    // 0x3C
    int     m_random_seed;      // 0x40
    short   m_mousedx;          // 0x44
    short   m_mousedy;          // 0x46
    bool    m_predicted;        // 0x48
    char    pad_0x49[0x18];     // 0x49 - 0x60 (size 100)
};

class CVerifiedUserCmd
{
public:
    CUserCmd            m_cmd;
    unsigned long       m_crc;
};

// Updated CInput for CS:GO 2014-10-23 build (VClient016, client.dll 12MB)
// Analyzed from dlls/client.dll - m_pCommands at 0xEC, m_fCameraInThirdPerson at 0xA4
// Previous version had thirdperson at 0x9D and commands at 0xDE - now fixed
class CInput
{
public:
    char pad_0x0000[0xA4];                  // 0x00 - 0xA3
    bool m_fCameraInThirdPerson;            // 0xA4 - confirmed via cmp byte ptr [esi+0xA4],0 in multiple funcs
    bool m_fCameraMovingWithMouse;          // 0xA5
    char pad_0x00A6[0x2];                   // 0xA6 - 0xA7 padding
    QAngle m_vecCameraOffset;               // 0xA8 - 0xB3 (12 bytes) - used in thirdperson
    bool m_fCameraDistanceMove;             // 0xB4
    char pad_0x00B5[0x3];                   // 0xB5 - 0xB7 align
    int m_nCameraOldX;                      // 0xB8
    int m_nCameraOldY;                      // 0xBC
    int m_nCameraX;                         // 0xC0
    int m_nCameraY;                         // 0xC4
    bool m_CameraIsOrthographic;            // 0xC8
    char pad_0x00C9[0x3];                   // 0xC9 - 0xCB
    QAngle m_angPreviousViewAngles;         // 0xCC - 0xD7
    QAngle m_angPreviousViewAnglesTilt;     // 0xD8 - 0xE3
    float m_flLastForwardMove;              // 0xE4
    int m_nClearInputState;                 // 0xE8
    CUserCmd* m_pCommands;                  // 0xEC - confirmed via disasm mov esi,[edi+0xEC]
    CVerifiedUserCmd* m_pVerifiedCommands;  // 0xF0 - confirmed via mov eax,[edi+0xF0]

    CUserCmd* GetUserCmd(int sequence)
    {
        if (!m_pCommands)
            return nullptr;
        // 150 = 0x96 - max commands
        return &m_pCommands[sequence % 150];
    }

    CVerifiedUserCmd* GetVerifiedUserCmd(int sequence)
    {
        if (!m_pVerifiedCommands)
            return nullptr;
        return &m_pVerifiedCommands[sequence % 150];
    }
};

// Compile-time checks
static_assert(sizeof(CUserCmd) == 0x64, "CUserCmd size must be 100");
static_assert(offsetof(CInput, m_fCameraInThirdPerson) == 0xA4, "m_fCameraInThirdPerson must be 0xA4");
static_assert(offsetof(CInput, m_pCommands) == 0xEC, "m_pCommands must be 0xEC");
static_assert(offsetof(CInput, m_pVerifiedCommands) == 0xF0, "m_pVerifiedCommands must be 0xF0");
