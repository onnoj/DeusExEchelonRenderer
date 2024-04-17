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
  std::vector<std::shared_ptr<PLH::IHook>> UMeshDetours;
  namespace UMeshVTableFuncs
  {
    void(__thiscall *GetFrame)(ULodMesh* pThis, FVector* Verts, INT Size, FCoords Coords, AActor* Owner) = nullptr;
  }
  namespace ULodMeshFuncs
  {
    HookableFunction GetFrame = &ULodMesh::GetFrame;
  }

  struct UMeshOverride
  {
    void GetFrame(FVector* Verts, INT	Size, FCoords	Coords, AActor* Owner)
    {
      UMeshVTableFuncs::GetFrame(reinterpret_cast<ULodMesh*>(this), Verts, Size, Coords, Owner);
    }
  };

  struct ULodMeshOverride
  {
    void GetFrame(FVector* Verts, INT Size, FCoords Coords, AActor* Owner, INT& LODRequest)
    {
      auto ctx = g_ContextManager.GetContext();

      //Kept in case we want to intercept the animation frames...
      (reinterpret_cast<ULodMesh*>(this)->*ULodMeshFuncs::GetFrame)(Verts, Size, Coords, Owner, LODRequest);
    }
  };

  namespace UMeshVTableOverrides
  {
    void(UMeshOverride::* GetFrame)(FVector* Verts, INT Size, FCoords Coords, AActor* Owner) = &UMeshOverride::GetFrame;
  }
  namespace ULodMeshOverrides
  {
    auto GetFrame = &ULodMeshOverride::GetFrame;
  }

  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> UMeshMappedVTableFuncs[] = {
    {{(uint16_t)28, *(uint64_t*)&UMeshVTableOverrides::GetFrame}, (uint64_t)&UMeshVTableFuncs::GetFrame, {}},
  };
}

void InstallUMeshHacks()
{
  using namespace Hacks;

  UMeshDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&ULodMeshFuncs::GetFrame, *(uint64_t*)&ULodMeshOverrides::GetFrame, &ULodMeshFuncs::GetFrame.func64));
  for (auto& detour : UMeshDetours)
  {
    detour->hook();
  }

  TObjectIterator<UMesh> ObjectIt;
  auto aLodMesh = *ObjectIt;
  for (auto& func : UMeshMappedVTableFuncs)
  {
    auto funcDescriptor = std::get<0>(func);
    uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
    PLH::VFuncMap* origFuncMap = &std::get<2>(func);
    origFuncMap->clear();
    PLH::VFuncMap v = { funcDescriptor };
    auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(aLodMesh), v, origFuncMap);
    ptr->hook();
    *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
    UMeshDetours.push_back(std::move(ptr));
  }
}

void UninstallUMeshHacks()
{
  using namespace Hacks;
  for (auto& detour : UMeshDetours)
  {
    detour->unHook();
  }
  ULodMeshFuncs::GetFrame.Restore();
  UMeshDetours.clear();
}