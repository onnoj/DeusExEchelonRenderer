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
  bool APlayerPawnHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> APlayerPawnDetours;
  namespace APlayerPawnVTableFuncs
  {
    void(__thiscall* ProcessEvent)(APlayerPawn* pThis, UFunction* Function, void* Params, void* Result) = nullptr;
  }
  namespace APlayerPawnFuncs
  {
  }

  struct APlayerPawnOverride
  {
    void ProcessEvent(UFunction* Function, void* Params, void* Result)
    {
      APlayerPawn* self = reinterpret_cast<APlayerPawn*>(this);
      std::wstring fn = Function->GetFullName();
      std::wstring on = self->GetFullName();
      static void* postRenderFlashFunction = [&]() {
        return self->FindFunctionChecked(ENGINE_PostRenderFlash, 1);
      }();
      if (Function == postRenderFlashFunction)
      {
        auto ctx = g_ContextManager.GetContext();
        Misc::g_Facade->GetHLRenderer()->OnDrawUIBegin(ctx->frameSceneNode);
      }
      APlayerPawnVTableFuncs::ProcessEvent(self, Function, Params, Result);
      if (Function == postRenderFlashFunction)
      {
        auto ctx = g_ContextManager.GetContext();
        Misc::g_Facade->GetHLRenderer()->OnDrawUIEnd(ctx->frameSceneNode);
      }
    }
  };

  namespace APlayerPawnVTableOverrides
  {
    void(APlayerPawnOverride::*ProcessEvent)(UFunction* Function, void* Params, void* Result) = &APlayerPawnOverride::ProcessEvent;
  }

  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> APlayerPawnMappedVTableFuncs[] = {
    {{(uint16_t)4, *(uint64_t*)&APlayerPawnVTableOverrides::ProcessEvent}, (uint64_t)&APlayerPawnVTableFuncs::ProcessEvent, {}},
  };
}

void InstallAPlayerPawnHacks()
{
  using namespace Hacks;

  if (!APlayerPawnHacksInstalled)
  {
    APlayerPawnHacksInstalled = true;
    //APlayerPawnDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&APlayerPawnFuncs::EventNotifyLevelChange, *(uint64_t*)&APlayerPawnVTableOverrides::eventNotifyLevelChange, &APlayerPawnFuncs::EventNotifyLevelChange.func64));
    //for (auto& detour : APlayerPawnDetours)
    //{
    //  detour->hook();
    //}
    
    TObjectIterator<APlayerPawn> ObjectIt;
    auto obj = *ObjectIt;
    for (auto& func : APlayerPawnMappedVTableFuncs)
    {
      auto funcDescriptor = std::get<0>(func);
      uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
      PLH::VFuncMap* origFuncMap = &std::get<2>(func);
      origFuncMap->clear();
      PLH::VFuncMap v = { funcDescriptor };
      auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(obj), v, origFuncMap);
      ptr->hook();
      *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
      APlayerPawnDetours.push_back(std::move(ptr));
    }
  }
}

void UninstallAPlayerPawnHacks()
{
  using namespace Hacks;

  if (APlayerPawnHacksInstalled)
  {
    APlayerPawnHacksInstalled = false;
    for (auto& detour : APlayerPawnDetours)
    {
      detour->unHook();
    }
    //APlayerPawnFuncs::EventNotifyLevelChange.Restore();
    APlayerPawnDetours.clear();
  }
}