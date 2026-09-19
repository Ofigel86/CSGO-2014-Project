#pragma once
#include "../interfaces/interfaces.hpp"

class c_ragebot
{
	int target_index = -1;
public:
	void instance(CUserCmd* command);
	void target_selection();
	Vector get_aimbot_point(c_cs_player* target);
	QAngle get_aimbot_angle(Vector source, Vector destination);
	void aim_at_target(CUserCmd* command, c_cs_player* target);
	void fire(CUserCmd* command);

	void auto_scope(CUserCmd* command);

	bool is_point_visible(c_cs_player* target, Vector source, Vector point);
	float get_wall_damage(Vector start, Vector end);
	bool hit_chance(CUserCmd* command);
};

extern c_ragebot* g_ragebot;