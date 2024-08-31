#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "utils/utils.h"
#include "utils/debugmenu.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameContextManager g_ContextManager;
std::deque<FrameContextManager::Context> FrameContextManager::m_stack;

void FrameContextManager::PushFrameContext()
{
  auto currentContext = GetContext();
  assert(m_stack.size() < 50);
  m_stack.push_back(currentContext ? FrameContextManager::Context(*currentContext) : FrameContextManager::Context{});
}

void FrameContextManager::PopFrameContext()
{
  m_stack.pop_back();
}

FrameContextManager::Context* FrameContextManager::GetContext()
{
  if (!m_stack.empty())
  {
    return &(m_stack.back());
  }
  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void Stats::OnTick()
{
  
  static auto lastSceneCount = 0;
  static auto lastskyBoxCount = 0;
  static auto lastmainFrameCount = 0;
  static bool printStatsToLog = false;
  g_DebugMenu.DebugVar("Rendering", "Print render stats to log", DebugMenuUniqueID(), printStatsToLog);
  if (printStatsToLog && (lastSceneCount != sceneCount || lastskyBoxCount != skyBoxCount || lastmainFrameCount != mainFrameCount))
  {
    lastSceneCount = sceneCount;
    lastskyBoxCount = skyBoxCount;
    lastmainFrameCount = mainFrameCount;
    GLog->Logf(L"sceneCount: %d\tskyboxCount: %d\tmainframeCount: %d, drawCallCount: %d (%d/%d/%d)\n",
      sceneCount,
      skyBoxCount,
      mainFrameCount,
      drawCallCount,
      sceneDrawCallCount[0],
      sceneDrawCallCount[1],
      sceneDrawCallCount[2]
    );
  }

  sceneCount = 0;
  drawCallCount = 0;
  skyBoxCount = 0;
  mainFrameCount = 0;
}

void Stats::BeginFrame()
{
  frameCount++;
  ZeroMemory(&sceneDrawCallCount[0], sizeof(sceneDrawCallCount));
}

void Stats::EndFrame()
{
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
