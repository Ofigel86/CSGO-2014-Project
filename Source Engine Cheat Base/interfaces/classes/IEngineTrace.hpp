#pragma once
#include "../../math/Vector4D.hpp"
#include "../../math/Vector.hpp"
#include "../../math/VMatrix.hpp"
#include "../../game/enums/bsp_flags.hpp"
#include "../../utilities/other/vfunc.hpp"
#include "IClientEntity.hpp"
#include "ClientClass.hpp"
#include <windows.h>

struct Ray_t
{
	VectorAligned        m_Start;
	VectorAligned        m_Delta;
	VectorAligned        m_StartOffset;
	VectorAligned        m_Extents;

	const matrix3x4_t* m_WorldAxisTransform;

	bool                m_IsRay;
	bool                m_IsSwept;

	Ray_t(void)
		: m_WorldAxisTransform(0)
	{

	}

	void Init(const Vector& start, const Vector& end)
	{
		m_Delta = end - start;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.Init();

		m_WorldAxisTransform = 0;

		m_IsRay = true;

		m_StartOffset.Init();

		m_Start = start;
	}

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs)
	{
		m_Delta = end - start;

		m_WorldAxisTransform = NULL;
		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents = maxs - mins;
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		m_StartOffset = maxs + mins;
		m_StartOffset *= 0.5f;
		m_Start = start + m_StartOffset;
		m_StartOffset *= -1.0f;
	};
};

class CBaseTrace
{
public:
	Vector                  startpos;
	Vector                  endpos;
	cplane_t                plane;
	float                   fraction;
	int                             contents;
	unsigned short  dispFlags;
	bool                    allsolid;
	bool                    startsolid;
};

struct csurface_t
{
	const char* name;
	short                   surfaceProps;
	unsigned short  flags;
};

class CGameTrace : public CBaseTrace
{
public:
	bool                    DidHitWorld() const;
	bool                    DidHitNonWorldEntity() const;
	int                             GetEntityIndex() const;
	bool                    DidHit() const;
public:
	float                   fractionleftsolid;
	csurface_t              surface;
	int                             hitgroup;
	short                   physicsbone;
	unsigned short  worldSurfaceIndex;
	IClientEntity* m_pEnt;
	int                             hitbox;
};

inline bool CGameTrace::DidHit() const
{
	return fraction < 1.0f || allsolid || startsolid;
}

typedef CGameTrace trace_t;

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,
	TRACE_ENTITIES_ONLY,
	TRACE_EVERYTHING_FILTER_PROPS,
};

class ITraceFilter
{
public:
	virtual bool            ShouldHitEntity(IClientEntity* pEntity, int contentsMask) = 0;
	virtual TraceType_t     GetTraceType() const = 0;
};

class CTraceFilter : public ITraceFilter
{
public:
	bool ShouldHitEntity(IClientEntity* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip;
};

class CTraceFilter2 : public ITraceFilter
{
public:
	bool ShouldHitEntity(IClientEntity* pEntityHandle, int contentsMask)
	{
		ClientClass* pEntCC = ((IClientEntity*)pEntityHandle)->GetClientClass();
		if (pEntCC && strcmp(ccIgnore, ""))
		{
			if (pEntCC->m_pNetworkName == ccIgnore)
				return false;
		}

		return !(pEntityHandle == pSkip);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip;
	const char* ccIgnore = "";
};

class CTraceFilterSkipTwoEntities : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity(IClientEntity* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip1 || pEntityHandle == pSkip2);
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
	void* pSkip2;
};

class IEngineTrace
{
public:
	virtual int   GetPointContents(const Vector& vecAbsPosition, int contentsMask = MASK_ALL, IHandleEntity** ppEntity = nullptr) = 0;
	virtual int   GetPointContents_WorldOnly(const Vector& vecAbsPosition, int contentsMask = MASK_ALL) = 0;
	virtual int   GetPointContents_Collideable(ICollideable* pCollide, const Vector& vecAbsPosition) = 0;
	virtual void  ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, CGameTrace* pTrace) = 0;
	virtual void  ClipRayToCollideable(const Ray_t& ray, unsigned int fMask, ICollideable* pCollide, CGameTrace* pTrace) = 0;
	virtual void  TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace) = 0;
};

struct mstudiobbox_t
{
	int     bone;
	int     group;
	Vector  bbmin;
	Vector  bbmax;
	int     szhitboxnameindex;
	int     unused[8];

	char* GetHitboxName(void)
	{
		if (szhitboxnameindex == 0)
			return (char*)"";

		return ((char*)this) + szhitboxnameindex;
	}
};

struct mstudiohitboxset_t
{
	int                                             sznameindex;
	inline char* const              GetName(void) const { return ((char*)this) + sznameindex; }
	int                                             numhitboxes;
	int                                             hitboxindex;
	inline mstudiobbox_t* GetHitbox(int i) const { return (mstudiobbox_t*)(((byte*)this) + hitboxindex) + i; };
};

struct mstudiobone_t
{
	int                                     sznameindex;
	inline char* const GetName(void) const { return ((char*)this) + sznameindex; }
	int                                     parent;
	int                                     bonecontroller[6];

	Vector                          pos;
	float                           quat[4];
	Vector                          rot;
	Vector                          posscale;
	Vector                          rotscale;

	matrix3x4_t                     poseToBone;
	float                           qAlignment[4];
	int                                     flags;
	int                                     proctype;
	int                                     procindex;              // procedural rule
	mutable int                     physicsbone;    // index into physically simulated bone
	inline void* GetProcedure() const { if (procindex == 0) return NULL; else return  (void*)(((byte*)this) + procindex); };
	int                                     surfacepropidx; // index into string tablefor property name
	inline char* const GetSurfaceProps(void) const { return ((char*)this) + surfacepropidx; }
	int                                     contents;               // See BSPFlags.h for the contents flags

	int                                     unused[8];              // remove as appropriate
};

struct studiohdr_t
{
	int                                     id;
	int                                     version;

	int                                     checksum;

	char                            name[64];
	int                                     length;


	Vector                          eyeposition;

	Vector                          illumposition;

	Vector                          hull_min;
	Vector                          hull_max;

	Vector                          view_bbmin;
	Vector                          view_bbmax;

	int                                     flags;

	int                                     numbones;
	int                                     boneindex;

	inline mstudiobone_t* GetBone(int i) const { return (mstudiobone_t*)(((byte*)this) + boneindex) + i; };
	//	inline mstudiobone_t *pBone(int i) const { Assert(i >= 0 && i < numbones); return (mstudiobone_t *)(((byte *)this) + boneindex) + i; };

	int                                     numbonecontrollers;
	int                                     bonecontrollerindex;

	int                                     numhitboxsets;
	int                                     hitboxsetindex;

	mstudiohitboxset_t* GetHitboxSet(int i) const
	{
		return (mstudiohitboxset_t*)(((byte*)this) + hitboxsetindex) + i;
	}

	inline mstudiobbox_t* GetHitbox(int i, int set) const
	{
		mstudiohitboxset_t const* s = GetHitboxSet(set);

		if (!s)
			return NULL;

		return s->GetHitbox(i);
	}

	inline int GetHitboxCount(int set) const
	{
		mstudiohitboxset_t const* s = GetHitboxSet(set);

		if (!s)
			return 0;

		return s->numhitboxes;
	}

	int                                     numlocalanim;
	int                                     localanimindex;

	int                                     numlocalseq;
	int                                     localseqindex;

	mutable int                     activitylistversion;
	mutable int                     eventsindexed;

	int                                     numtextures;
	int                                     textureindex;

	int                                     numcdtextures;
	int                                     cdtextureindex;

	int                                     numskinref;
	int                                     numskinfamilies;
	int                                     skinindex;

	int                                     numbodyparts;
	int                                     bodypartindex;

	int                                     numlocalattachments;
	int                                     localattachmentindex;

	int                                     numlocalnodes;
	int                                     localnodeindex;
	int                                     localnodenameindex;

	int                                     numflexdesc;
	int                                     flexdescindex;

	int                                     numflexcontrollers;
	int                                     flexcontrollerindex;

	int                                     numflexrules;
	int                                     flexruleindex;

	int                                     numikchains;
	int                                     ikchainindex;

	int                                     nummouths;
	int                                     mouthindex;

	int                                     numlocalposeparameters;
	int                                     localposeparamindex;

	int                                     surfacepropindex;

	int                                     keyvalueindex;
	int                                     keyvaluesize;


	int                                     numlocalikautoplaylocks;
	int                                     localikautoplaylockindex;

	float                           mass;
	int                                     contents;

	int                                     numincludemodels;
	int                                     includemodelindex;

	mutable void* virtualModel;

	int                                     szanimblocknameindex;
	int                                     numanimblocks;
	int                                     animblockindex;

	mutable void* animblockModel;

	int                                     bonetablebynameindex;

	void* pVertexBase;
	void* pIndexBase;

	byte                            constdirectionallightdot;

	byte                            rootLOD;

	byte                            numAllowedRootLODs;

	byte                            unused[1];

	int                                     unused4;

	int                                     numflexcontrollerui;
	int                                     flexcontrolleruiindex;
	float                           flVertAnimFixedPointScale;
	int                                     unused3[1];
	int                                     studiohdr2index;
	int                                     unused2[1];
};

struct surfacephysicsparams_t
{
	float    friction;
	float    elasticity; // collision elasticity - used to compute coefficient of restitution
	float    density;    // physical density (in kg / m^3)
	float    thickness;    // material thickness if not solid (sheet materials) in inches
	float    dampening;
};

struct surfaceaudioparams_t
{
	float    reflectivity;            // like elasticity, but how much sound should be reflected by this surface
	float    hardnessFactor;            // like elasticity, but only affects impact sound choices
	float    roughnessFactor;        // like friction, but only affects scrape sound choices   
	float    roughThreshold;            // surface roughness > this causes "rough" scrapes, < this causes "smooth" scrapes
	float    hardThreshold;            // surface hardness > this causes "hard" impacts, < this causes "soft" impacts
	float    hardVelocityThreshold;    // collision velocity > this causes "hard" impacts, < this causes "soft" impacts   
};

struct surfacesoundnames_t
{
	unsigned short    stepleft;
	unsigned short    stepright;
	unsigned short    impactSoft;
	unsigned short    impactHard;
	unsigned short    scrapeSmooth;
	unsigned short    scrapeRough;
	unsigned short    bulletImpact;
	unsigned short    rolling;
	unsigned short    breakSound;
	unsigned short    strainSound;
};

struct surfacegameprops_t
{
public:
	float    maxSpeedFactor; //0x0000
	float    jumpFactor; //0x0004
	char    pad00[0x4]; //0x0008
	float    flPenetrationModifier; //0x000C
	float    flDamageModifier; //0x0010
	unsigned short    material; //0x0014
	char    pad01[0x3];

};//Size=0x0019

struct surfacedata_t
{
	surfacephysicsparams_t    physics;
	surfaceaudioparams_t    audio;
	surfacesoundnames_t        sounds;
	surfacegameprops_t        game;
};

class IPhysicsSurfaceProps
{
public:
	surfacedata_t* GetSurfaceData(int surfaceDataIndex)
	{
		typedef surfacedata_t* (__thiscall* fnGetSurfaceData)(void*, int);
		return call_virtual<fnGetSurfaceData>(this, 5)(this, surfaceDataIndex);
	}
};