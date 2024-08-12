#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "utils.h"

uint32_t frameCount = 0;
uint32_t sceneCount = 0;
uint32_t sceneCountTotal = 0;
uint32_t drawCallCount = 0;
uint32_t drawCallCountTotal = 0;

Stats g_Stats;

Stats& Stats::Writer() const
{
  const Stats* self = this;
  return *const_cast<Stats*>(self);
}

void Stats::BeginFrame()
{
  frameCount++;
  sceneCount = 0;
  drawCallCount = 0;
  skyBoxCount = 0;
  mainFrameCount = 0;
  ZeroMemory(&sceneDrawCallCount[0], sizeof(sceneDrawCallCount));
}

void Stats::EndFrame()
{
#if 0
  GLog->Logf(L"sceneCount: %d\tskyboxCount: %d\tmainframeCount: %d, drawCallCount: %d (%d/%d/%d)\n",
    sceneCount,
    skyBoxCount,
    mainFrameCount,
    drawCallCount,
    sceneDrawCallCount[0],
    sceneDrawCallCount[1],
    sceneDrawCallCount[2]
  );
#endif
}


void Stats::BeginScene()
{
  sceneID++;
  sceneCount++;
  sceneCountTotal++;
}

void Stats::EndScene()
{
  sceneID--;
}

void Stats::DrawMainFrame()
{
  mainFrameCount++;
}

void Stats::DrawCall()
{
  drawCallCount++;
  drawCallCountTotal++;
  sceneDrawCallCount[sceneID]++;
}

void Stats::DrawSkyBox()
{
  skyBoxCount++;
  skyBoxCountTotal++;
}
