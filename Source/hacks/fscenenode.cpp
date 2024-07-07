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
      g_DebugMenu.DebugVar("Frame", "Zoom", DebugMenuUniqueID(), Frame->Zoom, {DebugMenuValueOptions::editor::slider, -10.0f, 10.0f});
      g_DebugMenu.DebugVar("Frame", "ViewPlanes[0]", DebugMenuUniqueID(), Frame->ViewPlanes[0], {});
      g_DebugMenu.DebugVar("Frame", "ViewPlanes[1]", DebugMenuUniqueID(), Frame->ViewPlanes[1], {});
      g_DebugMenu.DebugVar("Frame", "ViewPlanes[2]", DebugMenuUniqueID(), Frame->ViewPlanes[2], {});
      g_DebugMenu.DebugVar("Frame", "ViewPlanes[3]", DebugMenuUniqueID(), Frame->ViewPlanes[3], {});

      g_DebugMenu.DebugVar("Frame", "ViewSides[0]", DebugMenuUniqueID(), Frame->ViewSides[0], {});
      g_DebugMenu.DebugVar("Frame", "ViewSides[1]", DebugMenuUniqueID(), Frame->ViewSides[1], {});
      g_DebugMenu.DebugVar("Frame", "ViewSides[2]", DebugMenuUniqueID(), Frame->ViewSides[2], {});
      g_DebugMenu.DebugVar("Frame", "ViewSides[3]", DebugMenuUniqueID(), Frame->ViewSides[3], {});

      g_DebugMenu.DebugVar("Frame", "X", DebugMenuUniqueID(), Frame->X, {DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4000});
      g_DebugMenu.DebugVar("Frame", "Y", DebugMenuUniqueID(), Frame->Y, {DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4000});
      g_DebugMenu.DebugVar("Frame", "XB", DebugMenuUniqueID(), Frame->XB, {DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4000});
      g_DebugMenu.DebugVar("Frame", "YB", DebugMenuUniqueID(), Frame->YB, {DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4000});
      g_DebugMenu.DebugVar("Frame", "FX", DebugMenuUniqueID(), Frame->FX, {DebugMenuValueOptions::editor::slider, -4000.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "FY", DebugMenuUniqueID(), Frame->FY, {DebugMenuValueOptions::editor::slider, -4000.0f, 2880.0f});
      g_DebugMenu.DebugVar("Frame", "FX15", DebugMenuUniqueID(), Frame->FX15, {DebugMenuValueOptions::editor::slider, -4000.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "FY15", DebugMenuUniqueID(), Frame->FY15, {DebugMenuValueOptions::editor::slider, -4000.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "FX2", DebugMenuUniqueID(), Frame->FX2, {DebugMenuValueOptions::editor::slider, -4000.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "FY2", DebugMenuUniqueID(), Frame->FY2, {DebugMenuValueOptions::editor::slider, -4000.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "PrjXM", DebugMenuUniqueID(), Frame->PrjXM, {DebugMenuValueOptions::editor::slider, -400.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "PrjXP", DebugMenuUniqueID(), Frame->PrjXP, {DebugMenuValueOptions::editor::slider, -400.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "PrjYM", DebugMenuUniqueID(), Frame->PrjYM, {DebugMenuValueOptions::editor::slider, -400.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "PrjYP", DebugMenuUniqueID(), Frame->PrjYP, {DebugMenuValueOptions::editor::slider, -400.0f, 4000.0f});
      g_DebugMenu.DebugVar("Frame", "Proj", DebugMenuUniqueID(), Frame->Proj);
      g_DebugMenu.DebugVar("Frame", "RProj", DebugMenuUniqueID(), Frame->RProj);
      g_DebugMenu.DebugVar("Frame", "Nearclip", DebugMenuUniqueID(), Frame->NearClip);
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