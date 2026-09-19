#pragma once

#include "../../math/Vector2D.hpp"
#include "../../math/Vector4D.hpp"
#include "../../math/VMatrix.hpp"
#include "IEngineTrace.hpp"
#include "KeyValues.hpp"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CViewSetup;
class CEngineSprite;
class IClientEntity;
class IMaterial;
struct model_t;
class IClientRenderable;
class ITexture;

typedef void* StudioDecalHandle_t;
struct studiohwdata_t;

struct DrawModelState_t
{
    studiohdr_t* m_pStudioHdr;
    studiohwdata_t* m_pStudioHWData;
    IClientRenderable* m_pRenderable;
    const matrix3x4_t* m_pModelToWorld;
    StudioDecalHandle_t     m_decals;
    int                     m_drawFlags;
    int                     m_lod;
};

struct DrawModelInfo_t
{
    studiohdr_t* m_pStudioHdr;
    studiohwdata_t* m_pHardwareData;
    StudioDecalHandle_t m_Decals;
    int				m_Skin;
    int				m_Body;
    int				m_HitboxSet;
    void* m_pClientEntity;
    int				m_Lod;
    void* m_pColorMeshes;
    bool			m_bStaticLighting;
    void*	m_LightingState;
};

struct ModelRenderInfo_t
{
    Vector origin;
    Vector angles;
    void* pRenderable;
    const model_t* pModel;
    const matrix3x4_t* pModelToWorld;
    const matrix3x4_t* pLightingOffset;
    const Vector* pLightingOrigin;
    int flags;
    int entity_index;
    int skin;
    int body;
    int hitboxset;
    unsigned short instance;

    ModelRenderInfo_t()
    {
        pModelToWorld = NULL;
        pLightingOffset = NULL;
        pLightingOrigin = NULL;
    }
};

//-----------------------------------------------------------------------------
// Flags used by DrawWorldLists
//-----------------------------------------------------------------------------
enum
{
    DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER = 0x001,
    DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER = 0x002,
    DRAWWORLDLISTS_DRAW_INTERSECTSWATER = 0x004,
    DRAWWORLDLISTS_DRAW_WATERSURFACE = 0x008,
    DRAWWORLDLISTS_DRAW_SKYBOX = 0x010,
    DRAWWORLDLISTS_DRAW_CLIPSKYBOX = 0x020,
    DRAWWORLDLISTS_DRAW_SHADOWDEPTH = 0x040,
    DRAWWORLDLISTS_DRAW_REFRACTION = 0x080,
    DRAWWORLDLISTS_DRAW_REFLECTION = 0x100,
    DRAWWORLDLISTS_DRAW_WORLD_GEOMETRY = 0x200,
    DRAWWORLDLISTS_DRAW_DECALS_AND_OVERLAYS = 0x400,
};

enum
{
    MAT_SORT_GROUP_STRICTLY_ABOVEWATER = 0,
    MAT_SORT_GROUP_STRICTLY_UNDERWATER,
    MAT_SORT_GROUP_INTERSECTS_WATER_SURFACE,
    MAT_SORT_GROUP_WATERSURFACE,

    MAX_MAT_SORT_GROUPS
};

//-----------------------------------------------------------------------------
// Leaf index
//-----------------------------------------------------------------------------
typedef unsigned short LeafIndex_t;
enum
{
    INVALID_LEAF_INDEX = (LeafIndex_t)~0
};

struct WorldListLeafData_t
{
    LeafIndex_t     leafIndex;    // 16 bits
    int16_t         waterData;
    uint16_t        firstTranslucentSurface;    // engine-internal list index
    uint16_t        translucentSurfaceCount;    // count of translucent surfaces+disps
};

struct WorldListInfo_t
{
    int                     m_ViewFogVolume;
    int                     m_LeafCount;
    bool                    m_bHasWater;
    WorldListLeafData_t*    m_pLeafDataList;
};

class IWorldRenderList /*: public IRefCounted*/
{
};

//-----------------------------------------------------------------------------
// Describes the fog volume for a particular point
//-----------------------------------------------------------------------------
struct VisibleFogVolumeInfo_t
{
    int            m_nVisibleFogVolume;
    int            m_nVisibleFogVolumeLeaf;
    bool        m_bEyeInFogVolume;
    float       m_flDistanceToWater;
    float       m_flWaterHeight;
    IMaterial*  m_pFogVolumeMaterial;
};

struct VPlane
{
    Vector        m_Normal;
    vec_t        m_Dist;
};
#define FRUSTUM_NUMPLANES    6
typedef VPlane Frustum[FRUSTUM_NUMPLANES];
//-----------------------------------------------------------------------------
// Vertex format for brush models
//-----------------------------------------------------------------------------
struct BrushVertex_t //-V690
{
    Vector        m_Pos;
    Vector        m_Normal;
    Vector        m_TangentS;
    Vector        m_TangentT;
    Vector2D    m_TexCoord;
    Vector2D    m_LightmapCoord;

private:
    BrushVertex_t(const BrushVertex_t& src);
};

//-----------------------------------------------------------------------------
// Visibility data for area portal culling
//-----------------------------------------------------------------------------
struct VisOverrideData_t
{
    Vector        m_vecVisOrigin;                    // The point to to use as the viewpoint for area portal backface cull checks.
    float        m_fDistToAreaPortalTolerance;    // The distance from an area portal before using the full screen as the viewable portion.
};


//-----------------------------------------------------------------------------
// interface for asking about the Brush surfaces from the client DLL
//-----------------------------------------------------------------------------

class IBrushSurface
{
public:
    // Computes texture coordinates + lightmap coordinates given a world position
    virtual void ComputeTextureCoordinate(Vector const& worldPos, Vector2D& texCoord) = 0;
    virtual void ComputeLightmapCoordinate(Vector const& worldPos, Vector2D& lightmapCoord) = 0;

    // Gets the vertex data for this surface
    virtual int  GetVertexCount() const = 0;
    virtual void GetVertexData(BrushVertex_t* pVerts) = 0;

    // Gets at the material properties for this surface
    virtual IMaterial* GetMaterial() = 0;
};


//-----------------------------------------------------------------------------
// interface for installing a new renderer for brush surfaces
//-----------------------------------------------------------------------------

class IBrushRenderer
{
public:
    // Draws the surface; returns true if decals should be rendered on this surface
    virtual bool RenderBrushModelSurface(IClientEntity* pBaseEntity, IBrushSurface* pBrushSurface) = 0;
};

enum OverrideType_t
{
    OVERRIDE_NORMAL = 0,
    OVERRIDE_BUILD_SHADOWS,
    OVERRIDE_DEPTH_WRITE,
};

enum MaterialVarFlags_t
{
    MATERIAL_VAR_DEBUG = (1 << 0),
    MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
    MATERIAL_VAR_NO_DRAW = (1 << 2),
    MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),

    MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
    MATERIAL_VAR_VERTEXALPHA = (1 << 5),
    MATERIAL_VAR_SELFILLUM = (1 << 6),
    MATERIAL_VAR_ADDITIVE = (1 << 7),
    MATERIAL_VAR_ALPHATEST = (1 << 8),
    MATERIAL_VAR_MULTIPASS = (1 << 9),
    MATERIAL_VAR_ZNEARER = (1 << 10),
    MATERIAL_VAR_MODEL = (1 << 11),
    MATERIAL_VAR_FLAT = (1 << 12),
    MATERIAL_VAR_NOCULL = (1 << 13),
    MATERIAL_VAR_NOFOG = (1 << 14),
    MATERIAL_VAR_IGNOREZ = (1 << 15),
    MATERIAL_VAR_DECAL = (1 << 16),
    MATERIAL_VAR_ENVMAPSPHERE = (1 << 17),
    MATERIAL_VAR_NOALPHAMOD = (1 << 18),
    MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19),
    MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
    MATERIAL_VAR_TRANSLUCENT = (1 << 21),
    MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
    MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23),
    MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
    MATERIAL_VAR_ENVMAPMODE = (1 << 25),
    MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
    MATERIAL_VAR_HALFLAMBERT = (1 << 27),
    MATERIAL_VAR_WIREFRAME = (1 << 28),

    // NOTE: Only add flags here that either should be read from
    // .vmts or can be set directly from client code. Other, internal
    // flags should to into the flag enum in IMaterialInternal.h
};

#define MAX_VIS_LEAVES    32
#define MAX_AREA_STATE_BYTES        32
#define MAX_AREA_PORTAL_STATE_BYTES 24

class IModelInfo
{
public:
    int	GetModelIndex(const char* name)
    {
        typedef int(__thiscall* oGetModelName)(PVOID, const char*);
        return call_virtual< oGetModelName >(this, 2)(this, name);
    }

    const char* GetModelName(const model_t* mod)
    {
        typedef const char* (__thiscall* oGetModelName)(PVOID, const model_t*);
        return call_virtual< oGetModelName >(this, 3)(this, mod);
    }

    studiohdr_t* GetStudioModel(const model_t* mod)
    {
        typedef studiohdr_t* (__stdcall* oGetStudiomodel)(const model_t*);
        return call_virtual<oGetStudiomodel>(this, 30)(mod);
    }
};

class IVRenderView
{
public:
    void SetBlend(float alpha)
    {
        typedef void(__thiscall* oDrawModelExecute)(PVOID, float);
        return call_virtual< oDrawModelExecute >(this, 4)(this, alpha);
    }

    void SetColorModulation(float const* colors)
    {
        typedef void(__thiscall* oDrawModelExecute)(PVOID, float const*);
        return call_virtual< oDrawModelExecute >(this, 6)(this, colors);
    }
};

class IMaterial
{
public:
    const char* GetName()
    {
        typedef const char* (__thiscall* oGetName)(PVOID);
        return call_virtual< oGetName >(this, 0)(this);
    }

    void SetMaterialVarFlag(MaterialVarFlags_t flag, bool value)
    {
        typedef void(__thiscall* oSetMatFlag)(PVOID, MaterialVarFlags_t, bool);
        return call_virtual< oSetMatFlag >(this, 29)(this, flag, value);
    }

    bool GetMaterialVarFlag(MaterialVarFlags_t flag)
    {
        typedef bool(__thiscall* oGetMatFlag)(PVOID, MaterialVarFlags_t);
        return call_virtual< oGetMatFlag >(this, 31)(this, flag);
    }

    void AlphaModulate(float a)
    {
        typedef void(__thiscall* oAlphaModulate)(PVOID, float);
        return call_virtual< oAlphaModulate >(this, 28)(this, a);
    }

    void ColorModulate(float r, float g, float b)
    {
        typedef void(__thiscall* oColorModulate)(PVOID, float, float, float);
        return call_virtual< oColorModulate >(this, 29)(this, r, g, b);
    }

    void IncrementReferenceCount(void)
    {
        typedef void(__thiscall* oIncrementReferenceCount)(PVOID);
        return call_virtual< oIncrementReferenceCount >(this, 12)(this);
    }
};

#define TEXTURE_GROUP_WORLD							          "World textures"
#define TEXTURE_GROUP_MODEL							          "Model textures"

class IMaterialSystem
{
public:
    IMaterial* FindMaterial(char const* pMaterialName, const char* pTextureGroupName, bool complain = true, const char* pComplainPrefix = NULL)
    {
        typedef IMaterial* (__thiscall* oFindMaterial)(PVOID, char const*, char const*, bool, char const*);
        return call_virtual< oFindMaterial >(this, 82)(this, pMaterialName, pTextureGroupName, complain, pComplainPrefix);
    }

    IMaterial* CreateMaterial(const char* pMaterialName, KeyValues* pVMTKeyValues)
    {
        typedef IMaterial* (__thiscall* oCreateMaterial)(PVOID, const char*, KeyValues*);
        return call_virtual<oCreateMaterial>(this, 81)(this, pMaterialName, pVMTKeyValues);
    }
};

struct ColorRGBExp32
{
    byte r, g, b;
    signed char exponent;
};

struct dlight_t
{
    int		flags;
    Vector	origin;
    float	radius;
    ColorRGBExp32	color;
    float	die;
    float	decay;
    float	minlight;
    int		key;
    int		style;
    Vector	m_Direction;
    float	m_InnerAngle;
    float	m_OuterAngle;
    float GetRadius() const
    {
        return radius;
    }
    float GetRadiusSquared() const
    {
        return radius * radius;
    }
    float IsRadiusGreaterThanZero() const
    {
        return radius > 0.0f;
    }
};

class IVEffects
{
public:
    dlight_t* CL_AllocDlight(int key)
    {
        typedef dlight_t* (__thiscall* OriginalFn)(PVOID, int);
        return call_virtual<OriginalFn>(this, 4)(this, key);
    }
    dlight_t* CL_AllocElight(int key)
    {
        typedef dlight_t* (__thiscall* OriginalFn)(PVOID, int);
        return call_virtual<OriginalFn>(this, 5)(this, key);
    }
    dlight_t* GetElightByKey(int key)
    {
        typedef dlight_t* (__thiscall* OriginalFn)(PVOID, int);
        return call_virtual<OriginalFn>(this, 8)(this, key);
    }
};

class IVModelRender
{
public:
    void ForcedMaterialOverride(IMaterial* material, OverrideType_t type = OVERRIDE_NORMAL, int idk = NULL)
    {
        typedef void(__thiscall* Fn)(void*, IMaterial*, OverrideType_t, int);
        return call_virtual<Fn>(this, 1)(this, material, type, idk);
    }

    bool IsForcedMaterialOverride()
    {
        typedef bool(__thiscall* Fn)(void*);
        return call_virtual<Fn>(this, 2)(this);
    }
};