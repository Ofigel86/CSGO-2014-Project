#pragma once

class CGlobalVarsBase
{
public:
	float     m_realtime;                     // 0x0000
	int       m_framecount;                   // 0x0004
	float     m_absoluteframetime;            // 0x0008
	float     m_absoluteframestarttimestddev; // 0x000C
	float     m_curtime;                      // 0x0010
	float     m_frametime;                    // 0x0014
	int       m_maxclients;                   // 0x0018
	int       m_tickcount;                    // 0x001C
	float     m_intervalpertick;              // 0x0020
};