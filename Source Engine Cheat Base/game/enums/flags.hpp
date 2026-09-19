#pragma once

enum entity_flags 
{
	FL_ONGROUND = (1 << 0),
	FL_DUCKING = (1 << 1),
	FL_AIMDUCKING = (1 << 2),
	FL_WATERJUMP = (1 << 3),
	FL_ONTRAIN = (1 << 4),
	FL_INRAIN = (1 << 5),
	FL_FROZEN = (1 << 6),
	FL_ATCONTROLS = (1 << 7),
	FL_CLIENT = (1 << 8),
	FL_FAKECLIENT = (1 << 9),
	MAX_ENTITYFLAGS
};