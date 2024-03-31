/*=============================================================================
	UnSGL.h: Header file for PowerVR SGL support.

	Copyright 1997 NEC Electronics Inc.
	Based on code copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Jayeson Lee-Steere from code by Tim Sweeney

=============================================================================*/
#ifndef UNSGL_H
#define UNSGL_H

#include "DDC.h"

#define PYR(n) ((n)*((n+1))/2)

#define UNSGL_ENABLE_SGL_CLIPPING_VALUE  TRUE
#define UNSGL_DISABLE_SGL_CLIPPING_VALUE TRUE

#define UNSGL_Z_START 1.0f
#define UNSGL_Z_STEP  1.0001f
#define UNSGL_START_SCALE_INVW 1.0f
#define UNSGL_CLEAR_SCALE_INVW 4.0f
#define UNSGL_SKY_SCALE_INVW (UNSGL_CLEAR_SCALE_INVW/128.0f)

#define UNSGL_MAX_PALETTE_SIZE 0x100

#define UNSGL_MAX_UV_RATIO 4
#define UNSGL_MIN_TEXTURE_DIMENSION 32
#define UNSGL_MIN_TEXTURE_DIMENSION_F 32.0f
#define UNSGL_MAX_TEXTURE_DIMENSION 256

#define UNSGL_INC_X_ADD 0x5556
#define UNSGL_INC_X_AND 0xAAAA
#define UNSGL_INC_Y_ADD 0xAAAB
#define UNSGL_INC_Y_AND 0x5555

#define UNSGL_RGB_TO_I_RED_SCALE_VALUE		0x4CCC	// 0.30 * 0x10000
#define UNSGL_RGB_TO_I_GREEN_SCALE_VALUE	0x970A	// 0.59 * 0x10000
#define UNSGL_RGB_TO_I_BLUE_SCALE_VALUE		0x1C28	// 0.11 * 0x10000

#define UNSGL_MAX_VERTICES 60

#define UNSGL_PCX_LIGHTMAP_GAMMA 1.0f

// Texture upload flags.
enum ESGLTextureFlags
{
	SF_NoScale			= 0x01, // Scale for precision adjust.
	SF_LightMap			= 0x02, // Texture is a light map.
	SF_FogMap			= 0x04, // Texture is a fog map.
	SF_ColorKey			= 0x08, // Color key (0) implimented as 4444 argb (1555 on PVR2).
	SF_AdditiveBlend	= 0x10, // One-one blend (simulated with an alpha blend on PCX2).
	SF_ModulationBlend	= 0x20,	// Modulation blend (simulated with an alpha blend on PCX2).
};

#define TEXTURE_FLAGS_SHIFT 58

// Stuff for simple polygons buffering.
#define SIMPLE_VERTEX_BUFFER_SIZE 100
#define SIMPLE_POLYFLAGS_MASK (PF_Translucent | PF_Modulated | PF_Masked | PF_Invisible | PF_RenderFog | PF_NoSmooth | PF_Gouraud)
#define RESET_SIMPLE_VERTEX_BUFFER() \
{ \
	SimpleVertexCount=0; \
	SimpleVertexPtr=SimpleVertexBuffer; \
	SimpleFaceCount=0; \
	SimpleFacePtr=SimpleFaceBuffer; \
}
#define FLUSH_SIMPLE_VERTEX_BUFFER() \
{ \
	if (!(SimplePolyFlags & PF_Flat)) \
	{ \
		sgltri_triangles(&SglContext,SimpleFaceCount,SimpleFaceBuffer,(SGLVERTEX *)SimpleVertexBuffer); \
		if (SimplePolyFlags & PF_RenderFog) \
		{ \
			int Count; \
			PVERTEX Vertex; \
			for (Count=SimpleVertexCount,Vertex=SimpleVertexBuffer;Count>0;Count--,Vertex++) \
				Vertex->u32Colour=Vertex->u32Specular; \
			int OrgFlags=SglContext.u32Flags; \
			SglContext.u32Flags&=~SGLTT_TEXTURE; \
			SglContext.u32Flags|=SGLTT_VERTEXTRANS; \
			sgltri_triangles(&SglContext,SimpleFaceCount,SimpleFaceBuffer,(SGLVERTEX *)SimpleVertexBuffer); \
			SglContext.u32Flags=OrgFlags; \
		} \
	} \
	else \
	{ \
		sgltri_quads(&SglContext,SimpleFaceCount,SimpleQuadFaceBuffer,(SGLVERTEX *)SimpleVertexBuffer); \
		if (SimplePolyFlags & PF_RenderFog) \
		{ \
			int Count; \
			PVERTEX Vertex; \
			for (Count=SimpleVertexCount,Vertex=SimpleVertexBuffer;Count>0;Count--,Vertex++) \
				Vertex->u32Colour=Vertex->u32Specular; \
			int OrgFlags=SglContext.u32Flags; \
			SglContext.u32Flags&=~SGLTT_TEXTURE; \
			SglContext.u32Flags|=SGLTT_VERTEXTRANS; \
			sgltri_quads(&SglContext,SimpleFaceCount,SimpleQuadFaceBuffer,(SGLVERTEX *)SimpleVertexBuffer); \
			SglContext.u32Flags=OrgFlags; \
		} \
	} \
	RESET_SIMPLE_VERTEX_BUFFER(); \
}
#define FLUSH_SIMPLE_VERTEX_BUFFER_IF_NOT_EMPTY() \
{ \
	if (SimpleFaceCount!=0) \
		FLUSH_SIMPLE_VERTEX_BUFFER(); \
}
#define SIMPLE_HANDLER_FOG_OFFSET		3
#define SIMPLE_HANDLER_NORMAL			0
#define SIMPLE_HANDLER_TRANSLUCENT		1
#define SIMPLE_HANDLER_MODULATED		2
#define SIMPLE_HANDLER_NORMAL_FOG		(SIMPLE_HANDLER_NORMAL + SIMPLE_HANDLER_FOG_OFFSET)
#define SIMPLE_HANDLER_TRANSLUCENT_FOG	(SIMPLE_HANDLER_TRANSLUCENT + SIMPLE_HANDLER_FOG_OFFSET)
#define SIMPLE_HANDLER_MODULATED_FOG	(SIMPLE_HANDLER_MODULATED + SIMPLE_HANDLER_FOG_OFFSET)

// Handy class for creating a SGLVERTEX color value.
class SGLColor
{
public:
	// Variables.
	union
	{
		struct
		{
#if __INTEL_BYTE_ORDER__
			BYTE B,G,R,A;
#else
			BYTE A,R,G,B;
#endif
		};
		struct
		{
			CHAR NormalU,NormalV;
		};
		DWORD D;
		BYTE Component[4];
	};

	SGLColor() {}
	SGLColor( BYTE InR, BYTE InG, BYTE InB )
	:	R(InR), G(InG), B(InB) {}
	SGLColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA )
	:	R(InR), G(InG), B(InB), A(InA) {}
	SGLColor( const FPlane& P )
	:	R(Clamp(appFloor(P.X*256),0,255))
	,	G(Clamp(appFloor(P.Y*256),0,255))
	,	B(Clamp(appFloor(P.Z*256),0,255))
	,	A(Clamp(appFloor(P.W*256),0,255))
	{}
};

typedef struct tagVERTEX
{
	float fX;
	float fY;
	union 
	{
		float fZ;
		float fMasterS;
	};
	float fInvW;
	union
	{
		sgl_uint32  u32Colour;
		sgl_uint32  Color;
	};
	union
	{
		sgl_uint32  u32Specular;
		float fMasterT;
	};
	union
	{
		float  fUOverW;
		float  fU;
	};
	union
	{
		float  fVOverW;
		float  fV;
	};
} VERTEX, *PVERTEX;


#ifdef DOSTATS
#define CLOCK(a) clock(a)
#define UNCLOCK(a) unclock(a)
#define STATS_INC(a) a++
#else
#define CLOCK(a)
#define UNCLOCK(a)
#define STATS_INC(a)
#endif

struct FSGLStats
{
	// Stat variables.
	INT DisplayCount;

	INT Palettes;
	INT Textures;
	INT LightMaps;
	INT FogMaps;
	INT Reloads;
	INT LightMaxColors;
	INT FogMaxColors;
	INT ComplexSurfs, ComplexPolys, GouraudPolys, Tiles;

	INT PaletteTime;
	INT TextureTime;
	INT LightMapTime;
	INT FogMapTime;
	INT ReloadTime;
	INT LightMaxColorTime;
	INT FogMaxColorTime;
	INT TextureLoadTime;
	INT ComplexSurfTime;
	INT GouraudPolyTime;
	INT TileTime;
	INT StartOfFrameTime;
	INT RenderFinishTime;
	INT RenderTime;

	INT LargestLMU,LargestLMV;
	INT LargestLMDim;
};

extern FSGLStats Stats;
extern FSGLStats LastStats;

class DLL_EXPORT USGLRenderDevice : public URenderDevice, public UDDC
{
	DECLARE_CLASS(USGLRenderDevice,URenderDevice,CLASS_Config)

	// This is the viewport we are rendering into.
	UViewport *Viewport;

	// Configurable options
	UBOOL	VertexLighting;
	UBOOL	FastUglyRefresh;
	BYTE	ColorDepth;
	BYTE	TextureDetailBias;

	// Used by Lock/Unlock functions.
	DWORD	LockFlags;
	FPlane	FlashScale;
	FPlane	FlashFog;

	// SGL related stuff.
	INT		InFrame;
	DWORD	CurrentFrame;

	// Tables
	int		*GammaTable;
	int		*IntensityAdjustTable;

	// For buffering of simple polygons.
	DWORD	SimplePolyFlags;
	QWORD	SimplePolyTextureCacheID;
	DWORD	SimplePolyHandler;
	INT		SimpleVertexCount;
	PVERTEX SimpleVertexPtr;
	INT		SimpleFaceCount;
	union
	{
		int		(*SimpleFacePtr)[3];
		int		(*SimpleQuadFacePtr)[4];
	};
	VERTEX	SimpleVertexBuffer[SIMPLE_VERTEX_BUFFER_SIZE];
	union
	{
		int		SimpleFaceBuffer[SIMPLE_VERTEX_BUFFER_SIZE-2][3];
		int		SimpleQuadFaceBuffer[SIMPLE_VERTEX_BUFFER_SIZE/4][4];
	};

	// This is used to adjust the z range.
	FLOAT	ScaleInvW;
	FLOAT	NormalScaleInvW;
	FLOAT	SkyScaleInvW;
	FLOAT	NoZInvW;

	// For keeping track of different textures.
	struct USGLTexInfo
	{
		QWORD		CacheID;
		SGLColor	MaxColor;
		int			SglTextureName;
		FLOAT		UScale,VScale;
		INT			Dimensions;
		DWORD		User;
	} PolyCTex, PolyVTex, BumpMapTex, MacroTex, LightMapTex, DetailTex, FogMapTex;

	// Keeps track of all the loaded textures.
	struct USGLCacheObject
	{
		// Identifies texture.
		QWORD		CacheID;
		// Information required by polygon handling code.
		SGLColor	MaxColor;
		int			SglTextureName;
		FLOAT		UScale,VScale;
		INT			Dimensions;
		BOOL		ReloadCandidate;
		// Other stuff.
		DWORD		LastFrameUsed;
		BOOL		NotLoadedAtFullSize;
		// Linked list stuff.
		USGLCacheObject *Previous;
		USGLCacheObject *Next;
	} *StartCacheObjectList, *EndCacheObjectList;

	// Stops us from doing more than one reload try per frame.
	DWORD LastFrameReloadAtFullSizeTried;

	// Default textures.
	int DefaultTextureMap,DefaultLightMap,DefaultFogMap;

	// Keeps track of all textures which need to be deleted, but can't yet
	// because they are being used by a previous frame.
	struct USGLDeletionQueueItem
	{
		DWORD LastFrameUsed;
		int SglTextureName;
		USGLDeletionQueueItem *Next;
	} *DeletionQueue;

	// Functions in TEXCNVRT.CPP
	BYTE GetSglMapType(DWORD TextureFlags,BOOL MipMapped);
	BYTE GetSglMapSize(int Dimensions);
	int  GetSglDataSize(int Dimensions,BOOL MipMapped);
	int  MipMapOffset(int Dimensions,int Level);
	int NumLevels(int Dimensions);
	void CreateSglPalette(WORD *SglPalette,FColor *SrcPalette,INT PaletteSize,DWORD TextureFlags,FColor& MaxColor);
	void ConvertTextureData(int Dimensions,int XSize,int YSize,
							int TargetXSize,int TargetYSize,
							WORD *Target,BYTE *Src,WORD *SglPalette);
	void ConvertLightMapData(int Dimensions,int XSize,int YSize,int XClamp,int YClamp,
							 WORD *Target,FRainbowPtr Src,FColor& MaxColor);
	void ConvertFogMapData(int Dimensions,int XSize,int YSize,int XClamp,int YClamp,
						   WORD *Target,FRainbowPtr Src,FColor& MaxColor);
	
	// Functions in TEXTURE.CPP
	void CreateDefaultTextures(void);
	void InitCaching(void);
	void ShutDownCaching(void);
	USGLCacheObject *GetObjectFromTexture(QWORD CacheID);
	void AddCacheObjectToStartOfList(USGLCacheObject *Object);
	void RemoveCacheObjectFromList(USGLCacheObject *Object);
	void UnloadSglTexture(DWORD LastFrameUsed, int SglTextureName);
	void UnloadAllTextures(void);
	void FreeQueuedTextures(void);
	BOOL LoadTexture(FTextureInfo &Texture,DWORD Flags,
					 USGLCacheObject *CacheObject,BOOL AttemptingReloadAtFullSize);
	void SetTexture(FTextureInfo& Texture,DWORD Flags,QWORD CacheID,
				    USGLTexInfo &Info);

	// Functions in MISC.CPP
	void UpdateGammaTables(float Gamma);

public:
	// Constructors.
    void StaticConstructor();

    // UObject interface.
    void PostEditChange();
	void ShutdownAfterError();

	// URender interface. Functions in UNSGL.CPP
	UBOOL Init( UViewport* InViewport, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );
	UBOOL SetRes( INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );
	void Exit();
	void Flush( UBOOL AllowPrecache );
	void Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData, INT* HitSize );
	void Unlock( UBOOL Blit );
    void DrawComplexSurface( FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet );
    void DrawGouraudPolygon( FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span );
    void DrawTile( FSceneNode* Frame, FTextureInfo& Info, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL,FLOAT VL, class FSpanBuffer* Span, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags );
    UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
    void Draw2DLine( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector P1, FVector P2 );
    void Draw2DPoint( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, FLOAT Z );
    void GetStats( TCHAR* Result );
    void ClearZ( FSceneNode* Frame );
    void PushHit( const BYTE* Data, INT Count );
    void PopHit( INT Count, UBOOL bForce );
    void ReadPixels( FColor* Pixels );
};

#endif // UNSGL_H