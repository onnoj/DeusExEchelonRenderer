/*==========================================================================================
	A3d_Unreal.h: Header file for A3D geometry additions to Unreal and Unreal Tournament
	Created for Aureal Semiconductor, Inc. by Scott T. Etherton and Micah Mason
==========================================================================================*/

#include "ia3dapi.h"
#include "RenderPrivate.h"

typedef struct A3D_GeomStatus_s
{
  BOOL          bJustInitialized;
  float         fUnitsPerMeter;
  float         fMaxNodeDist;
  DWORD         dwMaxPoly;
  DWORD         dwMaxReflect;
  float         fRefDelay;
  float         fRefGain;
  float         fTransmittance;
  float         fLeafTransDistSq;
  BOOL          bEnableOcclusion;
  BOOL          bEnableReflection;

  float         fPolyTooSmall;
  float         fPolyAlwaysKeep;
  float         fPolyAngleTooSmall;
  float         fPolyReflectSize;

  // only used if __LOADMATERIALS is defined for SNDDLL
  BOOL			bEnableMaterials;

} A3D_GeomStatus;


enum A3D_EQ_TYPE
{
	A3D_EQ_TYPE_DEFAULT = 0,
	A3D_EQ_TYPE_WATER,
	A3D_EQ_TYPE_MAX
};

void A3D_Update(FCoords &Coords);
void A3D_UnrealInit(LPA3D3 A3d, LPA3DLISTENER A3dListener);
void A3D_RenderAudioGeometry(FSceneNode *Frame);
void A3D_UnrealDestroy(void);
void A3D_UpdateSource(IA3dSource *pSrc, BOOL JustStarted, BOOL Transform);
void A3D_UnrealSetEq(A3D_EQ_TYPE Type);
UBOOL A3D_Exec(const TCHAR* Cmd, FOutputDevice& Ar, UBOOL Use3dHardware);

#ifdef _DEBUG
void A3d_TempGetPosition(FVector *V, LPA3DSOURCE s);
#endif

