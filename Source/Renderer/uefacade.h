#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include "hacks/misc.h"

#include "rendering/dxtexturemanager.h"
#include "rendering/llrenderer.h"
#include "rendering/hlrenderer.h"

class UD3D9FPRenderDevice:public URenderDevice
{
	DECLARE_CLASS(UD3D9FPRenderDevice, URenderDevice, CLASS_Config)
public:
	UD3D9FPRenderDevice();

	void StaticConstructor();
	LowlevelRenderer* GetLLRenderer() { return &m_LLRenderer; }
	HighlevelRenderer* GetHLRenderer() { return &m_HLRenderer; }
public:
	virtual void Tick(FLOAT DeltaTime) final;
protected: //implementations of URenderDevice class
	virtual UBOOL Init(UViewport* pInViewport,int32_t pWidth, int32_t pHeight, int32_t pColorBytes, UBOOL pFullscreen) final;
	virtual UBOOL SetRes(INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen) final;
	virtual void Exit() final;
	virtual void Flush(UBOOL AllowPrecache) final;
	virtual void Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData, INT* HitSize ) final;
	virtual void Unlock(UBOOL Blit ) final;
	virtual void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet ) final;
	virtual void DrawGouraudPolygon( FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span ) final;
	virtual void DrawTile( FSceneNode* Frame, FTextureInfo& Info, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, class FSpanBuffer* Span, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags ) final;
	virtual void Draw2DLine( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector P1, FVector P2 ) final;
	virtual void Draw2DPoint( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, FLOAT Z ) final;
	virtual void ClearZ( FSceneNode* Frame ) final;
	virtual void PushHit( const BYTE* Data, INT Count ) final;
	virtual void PopHit( INT Count, UBOOL bForce ) final;
	virtual void GetStats( TCHAR* Result ) final;
	virtual void ReadPixels( FColor* Pixels ) final;
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) final;
	virtual void SetSceneNode( FSceneNode* Frame ) final;
	virtual void PrecacheTexture( FTextureInfo& Info, DWORD PolyFlags ) final;
	virtual void EndFlash() final;
public:
  static std::mutex m_Lock; //share lock between instances
private:
	static LowlevelRenderer m_LLRenderer; //keep between reinitializations
	static HighlevelRenderer m_HLRenderer; //keep between reinitializations
};


class UBenchmark:public AInfo
{
  DECLARE_CLASS(UBenchmark, AInfo, 0)
public:
  UBenchmark();
};