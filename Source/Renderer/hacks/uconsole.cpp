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
  bool UConsoleHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> UConsoleDetours;
  namespace UConsoleVTableFuncs
  {
    //void(__thiscall* GetFrame)(UConsole* pThis, FVector* Verts, INT Size, FCoords Coords, AActor* Owner) = nullptr;
  }
  namespace UConsoleFuncs
  {
    HookableFunction EventNotifyLevelChange = &UConsole::eventNotifyLevelChange;
  }

  struct UConsoleOverride
  {
    void eventNotifyLevelChange()
    {
      (reinterpret_cast<UConsole*>(this)->*UConsoleFuncs::EventNotifyLevelChange)();
    }
  };

  namespace UConsoleVTableOverrides
  {
    void(UConsoleOverride::*eventNotifyLevelChange)() = &UConsoleOverride::eventNotifyLevelChange;
  }

  //std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> UConsoleMappedVTableFuncs[] = {
  //  {{(uint16_t)28, *(uint64_t*)&UConsoleVTableOverrides::GetFrame}, (uint64_t)&UConsoleVTableFuncs::GetFrame, {}},
  //};
}

void InstallUConsoleHacks()
{
  using namespace Hacks;

  if (!UConsoleHacksInstalled)
  {
    UConsoleHacksInstalled = true;
    UConsoleDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&UConsoleFuncs::EventNotifyLevelChange, *(uint64_t*)&UConsoleVTableOverrides::eventNotifyLevelChange, &UConsoleFuncs::EventNotifyLevelChange.func64));
    for (auto& detour : UConsoleDetours)
    {
      detour->hook();
    }

    #if 0
    TObjectIterator<UConsole> ObjectIt;
    auto uconsole = *ObjectIt;
    for (auto& func : UConsoleMappedVTableFuncs)
    {
      auto funcDescriptor = std::get<0>(func);
      uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
      PLH::VFuncMap* origFuncMap = &std::get<2>(func);
      origFuncMap->clear();
      PLH::VFuncMap v = { funcDescriptor };
      auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(uconsole), v, origFuncMap);
      ptr->hook();
      *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
      UConsoleDetours.push_back(std::move(ptr));
    }
    #endif
  }
}

void UninstallUConsoleHacks()
{
  using namespace Hacks;

  if (UConsoleHacksInstalled)
  {
    UConsoleHacksInstalled = false;
    for (auto& detour : UConsoleDetours)
    {
      detour->unHook();
    }
    UConsoleFuncs::EventNotifyLevelChange.Restore();
    UConsoleDetours.clear();
  }
}