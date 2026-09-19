#pragma once

#include "../../utilities/other/netvars_helper.hpp"

#include "../enums/flags.hpp"
#include "../enums/bones.hpp"
#include "../enums/item_definition.hpp"

class weapon_info
{
public:
	char pad_0x0000[0x180]; // 0x0000
	int clip_size; // 0x0180
	char pad_0x0184[0xFC];
	char* weapon_name; // 0x0280
	char pad_0x0284[0x67C]; // 0x0284
	char pad_0x067C[0x90]; // 0x067C
	int weapon_price; // 0x0990
	float armor_ratio; // 0x0994
	char pad_0x998[0x10]; // 0x0998
	int penetration; // 0x09A8
	int damage; // 0x09AC
	float range; // 0x09B0
	float range_modifier; // 0x09B4
};

class c_base_entity : public IClientEntity
{
public:
	NETVAR(Vector, m_origin, "CBaseEntity", "m_vecOrigin");
	NETVAR(float, m_simulation_time, "CBaseEntity", "m_flSimulationTime");
	NETVAR(bool, m_spotted, "CBaseEntity", "m_bSpotted");
	NETVAR(int, m_team, "CBaseEntity", "m_iTeamNum");
};

class c_base_attributable_item : public c_base_entity
{
public:
	NETVAR(int, m_item_definition_index, "CBaseAttributableItem", "m_iItemDefinitionIndex");
};

class c_base_combat_weapon : public c_base_attributable_item
{
public:
	NETVAR(float, m_next_primary_attack, "CBaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(float, m_next_secondary_attack, "CBaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(int, m_item_definition_index, "CBaseCombatWeapon", "m_iItemDefinitionIndex");
	NETVAR(int, m_clip, "CBaseCombatWeapon", "m_iClip1");
	NETVAR(bool, m_pin_pulled, "CBaseCSGrenade", "m_bPinPulled");
	NETVAR(float, m_throw_time, "CBaseCSGrenade", "m_fThrowTime");
	NETVAR(float, m_postpone_fire_ready_time, "CWeaponCSBase", "m_flPostponeFireReadyTime");

	bool is_grenade()
	{
		return (this->m_item_definition_index() == WEAPON_HEGRENADE || this->m_item_definition_index() == WEAPON_MOLOTOV || this->m_item_definition_index() == WEAPON_INCGRENADE || this->m_item_definition_index() == WEAPON_FLASHBANG || this->m_item_definition_index() == WEAPON_SMOKEGRENADE);
	}

	bool is_knife()
	{
		return (this->m_item_definition_index() == WEAPON_KNIFE || this->m_item_definition_index() == WEAPON_KNIFE_T || this->m_item_definition_index() == WEAPON_UNDEFINED || this->m_item_definition_index() >= WEAPON_BAYONET);
	}

	bool is_sniper()
	{
		return (this->m_item_definition_index() == WEAPON_SSG08 || this->m_item_definition_index() == WEAPON_SCAR20 || this->m_item_definition_index() == WEAPON_G3SG1 || this->m_item_definition_index() == WEAPON_AWP/* || this->m_item_definition_index() == WEAPON_AUG || this->m_item_definition_index() == WEAPON_SG556*/);
	}

	bool is_non_aim()
	{
		return is_knife() || is_grenade() || (this->m_item_definition_index() == WEAPON_C4);
	}

	float get_inaccuracy()
	{
		if (!this)
			return 0.0f;

		return call_virtual<float(__thiscall*)(void*)>(this, 458)(this);
	}

	float get_spread()
	{
		if (!this)
			return 0.0f;

		return call_virtual<float(__thiscall*)(void*)>(this, 459)(this);
	}

	void update_accuracy_penalty()
	{
		if (!this)
			return;

		call_virtual<void(__thiscall*)(void*)>(this, 460)(this);
	}

	weapon_info* get_weapon_data()
	{
		if (!this)
			return nullptr;

		using Fn = weapon_info*(__thiscall*)(void*);
		return call_virtual<Fn>(this, 430)(this);
	}
};

class c_base_player : public c_base_entity
{
public:
	NETVAR(int, m_flags, "CBasePlayer", "m_fFlags");
	NETVAR(int, m_health, "CBasePlayer", "m_iHealth");
	NETVAR(Vector, m_view_offset, "CBasePlayer", "m_vecViewOffset[0]");
	NETVAR(QAngle, m_aim_punch_angle, "CBasePlayer", "m_aimPunchAngle");
	NETVAR(QAngle, m_view_punch_angle, "CBasePlayer", "m_viewPunchAngle");
};

class c_cs_player : public c_base_player
{
public:
	NETVAR(bool, m_is_scoped, "CCSPlayer", "m_bIsScoped");
	NETVAR(float, m_flash_duration, "CCSPlayer", "m_flFlashDuration");
	NETVAR(bool, m_gun_game_immunity, "CCSPlayer", "m_bGunGameImmunity");
	NETVAR(bool, m_has_helmet, "CCSPlayer", "m_bHasHelmet");
	NETVAR(int, m_armor_value, "CCSPlayer", "m_ArmorValue");
	NETVAR(bool, m_has_defuser, "CCSPlayer", "m_bHasDefuser");
	NETVAR(IHandleEntity*, m_active_weapon, "CBaseCombatCharacter", "m_hActiveWeapon");

	bool is_alive()
	{
		if (!this)
			return false;

		return m_health() > 0;
	}

	Vector get_eye_position()
	{
		return m_origin() + m_view_offset();
	}

	Vector get_bone_position(int bone)
	{
		matrix3x4_t boneMatrix[128];

		if (SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, this->m_simulation_time()))
			return Vector(boneMatrix[bone][0][3], boneMatrix[bone][1][3], boneMatrix[bone][2][3]);

		return Vector(0, 0, 0);
	}
};

class c_c4 : public c_base_entity
{
public:
	NETVAR(float, m_defuse_count_down, "CPlantedC4", "m_flDefuseCountDown");
	NETVAR(int, m_bomb_defuser, "CPlantedC4", "m_hBombDefuser");
	NETVAR(float, m_c4_blow, "CPlantedC4", "m_flC4Blow");
	NETVAR(bool, m_bomb_defused, "CPlantedC4", "m_bBombDefused");
};