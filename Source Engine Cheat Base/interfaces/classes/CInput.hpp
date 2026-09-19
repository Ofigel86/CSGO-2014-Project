#pragma once
#include "../../math/Vector.hpp"
#include "../../math/QAngle.hpp"
#include "../../utilities/checksum/checksum_crc.hpp"

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
#define IN_ZOOM   (1 << 19)

class CUserCmd
{
public:
	CUserCmd()
	{
		memset(this, NULL, sizeof(*this)); //-V598
	};

	virtual ~CUserCmd()
	{

	};

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

	int     m_command_number;	 // 0x04 For matching server and client commands for debugging
	int     m_tickcount;		 // 0x08 the tick the client created this command
	QAngle  m_viewangles;		 // 0x0C Player instantaneous view angles.
	Vector  m_aimdirection;		 // 0x18
	float   m_forwardmove;		  // 0x24
	float   m_sidemove;			  // 0x28
	float   m_upmove;			 // 0x2C
	int     m_buttons;			 // 0x30 Attack button states
	char    m_impulse;			// 0x34
	int     m_weaponselect;		// 0x38 Current weapon id
	int     m_weaponsubtype;	// 0x3C
	int     m_random_seed;     // 0x40 For shared random functions
	short   m_mousedx;			// 0x44 mouse accum in x from create move
	short   m_mousedy;			 // 0x46 mouse accum in y from create move
	bool    m_predicted;		// 0x48 Client only, tracks whether we've predicted this command at least once
	char    pad_0x4C[0x18];		// 0x4C Current sizeof( usercmd ) =  100  = 0x64
};

class CVerifiedUserCmd
{
public:
	CUserCmd			m_cmd;
	unsigned long		m_crc;
};

class CInput
{
public:
	char pad_0x000[0x9D]; // 0x00
	bool m_fCameraInThirdPerson; // 0x9D
	bool m_bUUGG; // 0x9E
	bool m_bUGG2; // 0x9F
	QAngle m_vecCameraOffset; // 0xA0
	bool m_fCameraDistanceMove;
	int m_nCameraOldX;
	int m_nCameraOldY;
	int m_nCameraX;
	int m_nCameraY;
	bool m_CameraIsOrthographic;
	QAngle m_angPreviousViewAngles;
	QAngle m_angPreviousViewAnglesTilt;
	float m_flLastForwardMove;
	int m_nClearInputState;
	CUserCmd* m_pCommands;
	CVerifiedUserCmd* m_pVerifiedCommands;

	CUserCmd* GetUserCmd(int sequence)
	{
		return &m_pCommands[sequence % 150];
	}

	CVerifiedUserCmd* GetVerifiedUserCmd(int sequence)
	{
		return &m_pVerifiedCommands[sequence % 150];
	}
};
