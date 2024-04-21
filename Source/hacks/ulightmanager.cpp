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
  std::vector<std::shared_ptr<PLH::IHook>> ULightManagerDetours;
  namespace ULightManagerVTableFuncs
  {
    //void(__thiscall *GetFrame)(ULightManager* pThis, FVector* Verts, INT Size, FCoords Coords, AActor* Owner) = nullptr;
  }
  namespace ULightManagerFuncs
  {
    //void(__thiscall *AddLight)(void* pThis, AActor* Owner, AActor* Light) = nullptr;
    HookableFunction<void (UObject::*)(AActor* Owner, AActor* Light)> AddLight = nullptr;
  }

  struct ULightManagerOverride
  {
    void AddLight(AActor* Owner, AActor* Light)
    {
      (reinterpret_cast<ULodMesh*>(this)->*ULightManagerFuncs::AddLight)(Owner, Light);
    }
  };

  namespace ULightManagerVTableOverrides
  {
    //void(ULightManagerOverride::* GetFrame)(FVector* Verts, INT Size, FCoords Coords, AActor* Owner) = &ULightManagerOverride::GetFrame;
  }
  namespace ULightManagerOverrides
  {
    auto AddLight = &ULightManagerOverride::AddLight;
  }

  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> ULightManagerMappedVTableFuncs[];/* = {
    {{(uint16_t)28, *(uint64_t*)&ULightManagerVTableOverrides::GetFrame}, (uint64_t)&ULightManagerVTableFuncs::GetFrame, {}},
  };*/
}

void InstallULightManagerHacks()
{
  using namespace Hacks;

  auto base = reinterpret_cast<uint32_t>(GetModuleHandle(L"Render.dll"));
  ULightManagerFuncs::AddLight = (void*)(base + 0x1122);
  ULightManagerDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&ULightManagerFuncs::AddLight, *(uint64_t*)&ULightManagerOverrides::AddLight, (uint64_t*) & ULightManagerFuncs::AddLight));
  for (auto& detour : ULightManagerDetours)
  {
    detour->hook();
  }

  /*
  for (auto& func : ULightManagerMappedVTableFuncs)
  {
    auto funcDescriptor = std::get<0>(func);
    uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
    PLH::VFuncMap* origFuncMap = &std::get<2>(func);
    origFuncMap->clear();
    PLH::VFuncMap v = { funcDescriptor };
    auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(aLodMesh), v, origFuncMap);
    ptr->hook();
    *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
    ULightManagerDetours.push_back(std::move(ptr));
  }*/

}

void UninstallULightManagerHacks()
{
  using namespace Hacks;
  for (auto& detour : ULightManagerDetours)
  {
    detour->unHook();
  }
  //ULightManagerFuncs::GetFrame.Restore();
  ULightManagerDetours.clear();
}