#pragma once
#include "IClientEntity.hpp"
#include "../math/QAngle.hpp"
#include "../math/Vector.hpp"

class CUserCmd;

class IMoveHelper {
public:
	virtual void virtual000() = 0;
	virtual void SetHost(IClientEntity* host) = 0;
	virtual void virtual002() = 0;
	virtual void virtual003() = 0;
	virtual void ProcessImpacts() = 0;
};

class CMoveData {
public:
	bool			m_bFirstRunOfFunctions : 1;
	bool			m_bGameCodeMovedPlayer : 1;
	bool			m_bNoAirControl : 1;

	unsigned long	m_nPlayerHandle;
	int				m_nImpulseCommand;
	QAngle			m_vecViewAngles;
	QAngle			m_vecAbsViewAngles;
	int				m_nButtons;
	int				m_nOldButtons;
	float			m_flForwardMove;
	float			m_flSideMove;
	float			m_flUpMove;

	float			m_flMaxSpeed;
	float			m_flClientMaxSpeed;

	Vector			m_vecVelocity;
	Vector			m_vecOldVelocity;
	float			somefloat;
	QAngle			m_vecAngles;
	QAngle			m_vecOldAngles;

	float			m_outStepHeight;
	Vector			m_outWishVel;
	Vector			m_outJumpVel;

	Vector			m_vecConstraintCenter;
	float			m_flConstraintRadius;
	float			m_flConstraintWidth;
	float			m_flConstraintSpeedFactor;
	bool			m_bConstraintPastRadius;

	void			SetAbsOrigin(const Vector& vec);
	const Vector& GetAbsOrigin() const;

private:
	Vector			m_vecAbsOrigin;		// edict::origin
};

class IPrediction {
public:
	virtual void UnknownVirtual0() = 0;
	virtual void UnknownVirtual1() = 0;
	virtual void UnknownVirtual2() = 0;
	virtual void UnknownVirtual3() = 0;
	virtual void UnknownVirtual4() = 0;
	virtual void UnknownVirtual5() = 0;
	virtual void UnknownVirtual6() = 0;
	virtual void UnknownVirtual7() = 0;
	virtual void UnknownVirtual8() = 0;
	virtual void UnknownVirtual9() = 0;
	virtual void UnknownVirtual10() = 0;
	virtual void UnknownVirtual11() = 0;
	virtual void UnknownVirtual12() = 0;
	virtual void SetLocalViewAngles(QAngle& ang) = 0;
	virtual void UnknownVirtual14() = 0;
	virtual void UnknownVirtual15() = 0;
	virtual void UnknownVirtual16() = 0;
	virtual void UnknownVirtual17() = 0;
	virtual void CheckMovingGround(IClientEntity* player, float frametime) = 0;
	virtual void RunCommand(IClientEntity* player, CUserCmd* ucmd, IMoveHelper* pHelper) = 0;
	virtual void SetupMove(IClientEntity* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move) = 0; //20
	virtual void FinishMove(IClientEntity* player, CUserCmd* ucmd, CMoveData* move) = 0;
};

class IGameMovement {
public:
	virtual			~IGameMovement(void) {}

	virtual void	ProcessMovement(IClientEntity* pPlayer, CMoveData* pMove) = 0;
	virtual void	Reset(void) = 0;
	virtual void	StartTrackPredictionErrors(IClientEntity* pPlayer) = 0;
	virtual void	FinishTrackPredictionErrors(IClientEntity* pPlayer) = 0;
	virtual void	DiffPrint(char const* fmt, ...) = 0;

	virtual Vector const& GetPlayerMins(bool ducked) const = 0;
	virtual Vector const& GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector const& GetPlayerViewOffset(bool ducked) const = 0;

	virtual bool			IsMovingPlayerStuck(void) const = 0;
	virtual IClientEntity* GetMovingPlayer(void) const = 0;
	virtual void			UnblockPusher(IClientEntity* pPlayer, IClientEntity* pPusher) = 0;

	virtual void SetupMovementBounds(CMoveData* pMove) = 0;
};