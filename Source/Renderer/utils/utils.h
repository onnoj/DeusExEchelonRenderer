#pragma once

static D3DXVECTOR3 UEVecToD3DXVec3(const FVector& pV)
{
  return D3DXVECTOR3{pV.X, pV.Y, pV.Z};
}

static FVector D3DXVec3ToUEVec(const D3DXVECTOR3& pV)
{
  return FVector{pV.x, pV.y, pV.z};
}

static D3DVECTOR UEVecToD3DVec3(const FVector& pV)
{
  return D3DVECTOR{pV.X, pV.Y, pV.Z};
}

static FVector D3DVec3ToUEVec(const D3DVECTOR& pV)
{
  return FVector{pV.x, pV.y, pV.z};
}

static D3DVECTOR D3DVec3ToD3DXVec3(const D3DVECTOR& pV)
{
  return D3DVECTOR{pV.x, pV.y, pV.z};
}

static D3DVECTOR D3DXVec3ToD3DVec(const D3DXVECTOR3& pV)
{
  return D3DVECTOR{pV.x, pV.y, pV.z};
}

struct Stats
{
  void OnTick();

  void BeginFrame();
  void EndFrame();

  void BeginScene();
  void EndScene();

  void DrawCall();
  void DrawSkyBox();
  void DrawMainFrame();

  Stats& Writer() const;

  int32_t sceneID = -1;

  uint32_t skyBoxCount = 0;
  uint32_t skyBoxCountTotal = 0;
  uint32_t frameCount = 0;
  uint32_t sceneCount = 0;
  uint32_t sceneCountTotal = 0;
  uint32_t drawCallCount = 0;
  uint32_t drawCallCountTotal = 0;
  uint32_t sceneDrawCallCount[8]{ 0 };
  uint32_t mainFrameCount = 0;

} extern g_Stats;