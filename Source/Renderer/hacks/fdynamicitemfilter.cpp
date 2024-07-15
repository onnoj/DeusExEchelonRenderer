#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hacks.h"

#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool DynamicItemFilterHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> DynamicItemFilterDetours;
  namespace DynamicItemFilterVTableFuncs
  {
    void(__thiscall* Filter)(FDynamicItem* pThis, UViewport* Viewport, FSceneNode* Frame, INT iNode, INT Outside) = nullptr;
    void(__thiscall* PreRender)(FDynamicFinalChunk* pThis, UViewport* Viewport, FSceneNode* Frame, FSpanBuffer* SpanBuffer, INT iNode, FVolActorLink* Volumetrics);
  }
  class FakeDynamicItemFilter
  {
  public:
    void Filter(UViewport* Viewport, FSceneNode* Frame, INT iNode, INT Outside)
    {
      auto ctx = g_ContextManager.GetContext();
      if (!ctx->overrides.skipDynamicFiltering)
      {
        DynamicItemFilterVTableFuncs::Filter(reinterpret_cast<FDynamicItem*>(this), Viewport, Frame, iNode, Outside);
      }
      return;
    }
  };
  struct FakeFDynamicFinalChunk
  {
    void PreRender(UViewport* Viewport, FSceneNode* Frame, FSpanBuffer* SpanBuffer, INT iNode, FVolActorLink* Volumetrics)
    {
      DynamicItemFilterVTableFuncs::PreRender(reinterpret_cast<FDynamicFinalChunk*>(this), Viewport, Frame, SpanBuffer, iNode, Volumetrics);
    }
  };

  namespace DynamicItemFilterVTableOverrides
  {
    void(FakeDynamicItemFilter::* Filter)(UViewport* Viewport, FSceneNode* Frame, INT iNode, INT Outside) = &FakeDynamicItemFilter::Filter;
    void(FakeFDynamicFinalChunk::* PreRender)(UViewport* Viewport, FSceneNode* Frame, FSpanBuffer* SpanBuffer, INT iNode, FVolActorLink* Volumetrics) = &FakeFDynamicFinalChunk::PreRender;
  }

  /*
  * VTable for URender:
  *  [21] UGameEngine::Tick
  */
  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> FDynamicFinalChunkVTableFuncs[] = { {}
    /*{{(uint16_t)21, *(uint64_t*)&DynamicItemFilterVTableOverrides::PreRender}, (uint64_t)&DynamicItemFilterVTableFuncs::PreRender, {}},*/
  };
  /////////////////////////////////////////////////////
}

void InstallFDynamicItemFilterHacks()
{
#if 1
  using namespace Hacks;

  if (!DynamicItemFilterHacksInstalled)
  {
    DynamicItemFilterHacksInstalled = true;
    HMODULE renderModule = GetModuleHandleA("render.dll");
    auto origAddress = PVOID(uint64_t(renderModule) + 0x114A);
    DynamicItemFilterVTableFuncs::Filter = reinterpret_cast<decltype(DynamicItemFilterVTableFuncs::Filter)>(origAddress);
    DynamicItemFilterDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&DynamicItemFilterVTableFuncs::Filter, *(uint64_t*)&DynamicItemFilterVTableOverrides::Filter, (uint64_t*)&DynamicItemFilterVTableFuncs::Filter));
    for (auto& detour : DynamicItemFilterDetours)
    {
      detour->hook();
    }
  }
#endif
}

void UninstallFDynamicItemFilterHacks()
{
  using namespace Hacks;

  if (DynamicItemFilterHacksInstalled)
  {
    DynamicItemFilterHacksInstalled = false;
    for (auto& detour : DynamicItemFilterDetours)
    {
      detour->unHook();
    }
    DynamicItemFilterDetours.clear();
  }
}