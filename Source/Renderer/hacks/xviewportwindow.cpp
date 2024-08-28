#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "misc.h"
#include "hacks.h"
#include "uefacade.h"
#include "utils/debugmenu.h"
#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool XViewportWindowHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> XViewportWindowDetours;
  namespace XViewportWindowVTableFuncs
  {
    void(__thiscall* RenderFrame)(XViewportWindow* pThis, class URenderBase* render, struct FSceneNode* frame) = nullptr;
  }
  namespace XViewportWindowFuncs
  {
  }

  struct XViewportWindowOverride
  {
    void RenderFrame(class URenderBase* render, struct FSceneNode* frame)
    {
      XViewportWindowVTableFuncs::RenderFrame(reinterpret_cast<XViewportWindow*>(this), render, frame);
    }
  };

  namespace XViewportWindowVTableOverrides
  {
    void(XViewportWindowOverride::*RenderFrame)(class URenderBase* render, struct FSceneNode* frame) = &XViewportWindowOverride::RenderFrame;
  }

  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> XViewportWindowMappedVTableFuncs[] = {
    {{(uint16_t)0x48, *(uint64_t*)&XViewportWindowVTableOverrides::RenderFrame}, (uint64_t)&XViewportWindowVTableFuncs::RenderFrame, {}},
  };
}

void InstallXViewportWindowHacks()
{
  using namespace Hacks;

  if (!XViewportWindowHacksInstalled)
  {
    XViewportWindowHacksInstalled = true;
    //XViewportWindowDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&XViewportWindowFuncs::EventNotifyLevelChange, *(uint64_t*)&XViewportWindowVTableOverrides::eventNotifyLevelChange, &XViewportWindowFuncs::EventNotifyLevelChange.func64));
    //for (auto& detour : XViewportWindowDetours)
    //{
    //  detour->hook();
    //}

    auto obj = ConstructObject<XViewportWindow>(XViewportWindow::StaticClass());
    for (auto& func : XViewportWindowMappedVTableFuncs)
    {
      auto funcDescriptor = std::get<0>(func);
      uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
      PLH::VFuncMap* origFuncMap = &std::get<2>(func);
      origFuncMap->clear();
      PLH::VFuncMap v = { funcDescriptor };
      auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(obj), v, origFuncMap);
      ptr->hook();
      *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
      XViewportWindowDetours.push_back(std::move(ptr));
    }
    delete(obj); obj = nullptr;
  }
}

void UninstallXViewportWindowHacks()
{
  using namespace Hacks;

  if (XViewportWindowHacksInstalled)
  {
    XViewportWindowHacksInstalled = false;
    for (auto& detour : XViewportWindowDetours)
    {
      detour->unHook();
    }
    //XViewportWindowFuncs::EventNotifyLevelChange.Restore();
    XViewportWindowDetours.clear();
  }
}