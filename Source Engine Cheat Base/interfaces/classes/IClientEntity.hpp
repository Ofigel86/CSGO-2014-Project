#pragma once 

#include "IClientNetworkable.hpp"
#include "IClientRenderable.hpp"
#include "IClientUnknown.hpp"
#include "IClientThinkable.hpp"

struct SpatializationInfo_t;

class Vector;
class QAngle;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	// Delete yourself.
	virtual void			Release( void ) = 0;

	// Network origin + angles
	virtual const Vector&	GetAbsOrigin( void ) const = 0;
	virtual const QAngle&	GetAbsAngles( void ) const = 0;
};