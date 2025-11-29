#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hacks.h"
#include "utils/debugmenu.h"
#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool FSceneNodeHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> FSceneNodeDetours;
  namespace FSceneNodeFuncs
  {
    HookableFunction ComputeRenderSize = &::FSceneNode::ComputeRenderSize;
  }

  struct FSceneNodeOverride
  {
    void ComputeRenderSize()
    {
      FSceneNode* Frame = reinterpret_cast<FSceneNode*>(this);
      (Frame->*FSceneNodeFuncs::ComputeRenderSize)();
      g_DebugMenu.DebugUEFrame("Frame", Frame);
    }
  };

  namespace FSceneNodeOverrides
  {
    auto ComputeRenderSize = &FSceneNodeOverride::ComputeRenderSize;
  }
}

void InstallFSceneNodeHacks()
{
  using namespace Hacks;

  if (!FSceneNodeHacksInstalled)
  {
    FSceneNodeHacksInstalled = true;
    FSceneNodeDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSceneNodeFuncs::ComputeRenderSize, *(uint64_t*)&FSceneNodeOverrides::ComputeRenderSize, &FSceneNodeFuncs::ComputeRenderSize.func64));
    for (auto& detour : FSceneNodeDetours)
    {
      detour->hook();
    }
  }

}

void UninstallFSceneNodeHacks()
{
  using namespace Hacks;

  if (FSceneNodeHacksInstalled)
  {
    FSceneNodeHacksInstalled = false;
    FSceneNodeDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSceneNodeFuncs::ComputeRenderSize, *(uint64_t*)&FSceneNodeOverrides::ComputeRenderSize, &FSceneNodeFuncs::ComputeRenderSize.func64));
    for (auto& detour : FSceneNodeDetours)
    {
      detour->unHook();
    }
    FSceneNodeFuncs::ComputeRenderSize.Restore();
    FSceneNodeDetours.clear();
  }
}