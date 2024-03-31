#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "misc.h"
#include "hacks.h"
#include "uefacade.h"
#include "utils/debugmenu.h"
#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>
#include <deusex/ConSys/Inc/ConSys.h>
#include <deusex/ConSys/Inc/ConCamera.h>

namespace Hacks
{  
  std::vector<std::shared_ptr<PLH::IHook>> UGameEngineDetours;
  UGameEngine* GEngine = nullptr;
  namespace UGameEngineVTableFuncs
  {
    void(__thiscall *Tick)(UGameEngine* pThis, FLOAT DeltaSeconds) = nullptr;
  }

  struct UGameEngineOverride
  {
    void Tick(FLOAT DeltaSeconds)
    {
      UGameEngineVTableFuncs::Tick(GEngine, DeltaSeconds);
      ::Misc::g_Facade->Tick(DeltaSeconds);

      for (TObjectIterator<ADeusExPlayer> objs; objs; objs.operator++())
      {
        auto obj = *objs;
        if (obj->ConPlay)
        {
          static bool lastIsInCutscene = false;
          bool isInCutscene = *(((char*)obj->ConPlay) + 41)!=0;
          if (isInCutscene != lastIsInCutscene)
          {
            int x = 1;
          }
          lastIsInCutscene = isInCutscene;
        }
        int x = 1;
      }

    }
  };

  namespace UGameEngineVTableOverrides
  {
    void(UGameEngineOverride::* Tick)(FLOAT DeltaSeconds) = &UGameEngineOverride::Tick;
   
  }

  /*
  * VTable for URender:
  *  [21] UGameEngine::Tick
  */
  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> UGameEngineMappedVTableFuncs[] = {
    {{(uint16_t)21, *(uint64_t*)&UGameEngineVTableOverrides::Tick}, (uint64_t)&UGameEngineVTableFuncs::Tick, {}},
  };
}

void InstallUGameEngineHacks()
{
  using namespace Hacks;
  for (auto& detour : UGameEngineDetours)
  {
    detour->hook();
  }

  TObjectIterator<UGameEngine> EngineIt;
  GEngine = *EngineIt;
  for (auto& func : UGameEngineMappedVTableFuncs)
  {
    auto funcDescriptor = std::get<0>(func);
    uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
    PLH::VFuncMap* origFuncMap = &std::get<2>(func);
    origFuncMap->clear();
    PLH::VFuncMap v = { funcDescriptor };
    auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(GEngine), v, origFuncMap);
    ptr->hook();
    *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
    UGameEngineDetours.push_back(std::move(ptr));
  }
}

void UninstallUGameEngineHacks()
{
  using namespace Hacks;
  for (auto& detour : UGameEngineDetours)
  {
    detour->unHook();
  }
  UGameEngineDetours.clear();
}