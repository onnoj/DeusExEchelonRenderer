#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hacks.h"

#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool FDynamicSpriteHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> FDynamicSpriteDetours;
  namespace FDynamicSpriteVTableFuncs
  {
  }
  namespace FDynamicSpriteFuncs
  {
    HookableFunction<UBOOL(__thiscall FDynamicSprite::*)(FSceneNode* Frame)> Setup = nullptr;
    HookableFunction<FDynamicSprite*(__thiscall FDynamicSprite::*)(FSceneNode* Frame, int NodeIndex, AActor* Actor)> Constructor0 = nullptr;
  }
  class FakeFDynamicSprite
  {
  public:
    UBOOL __thiscall Setup(FSceneNode* Frame)
    {
      auto ctx = g_ContextManager.GetContext();
      //if (ctx->overrides.disableFDynamicSpriteSetup)
      //{
      //  return FALSE;
      //}
      //AActor* actor = *reinterpret_cast<AActor**>((reinterpret_cast<uint8_t*>(this) + 0x94));
      //auto res = (reinterpret_cast<FDynamicSprite*>(this)->*FDynamicSpriteFuncs::Setup)(Frame);
      //if (!res && actor->DrawType == DT_Mesh)
      //{
      //  if (actor == Frame->Viewport->Actor)
      //  {
      //    return TRUE;
      //  }
      //}
      //return res;
      return (reinterpret_cast<FDynamicSprite*>(this)->*FDynamicSpriteFuncs::Setup)(Frame);
    }

    FDynamicSprite* __thiscall Constructor0(FSceneNode* Frame, int NodeIndex, AActor* Actor)
    {
      //FrameContextManager::ScopedContext ctx;
      //ctx->overrides.disableFDynamicSpriteSetup = true;

      //TODO: enable collection of rejected actors, and in ::Setup, collect rejected actors.
      //TODO: then, at the end of the frame, draw all actors with DrawActor.
      return (reinterpret_cast<FDynamicSprite*>(this)->*FDynamicSpriteFuncs::Constructor0)(Frame, NodeIndex, Actor);
    }
  };

  namespace FDynamicSpriteVTableOverrides
  {
  }
  namespace FDynamicSpriteOverrides
  {
    auto Setup = &FakeFDynamicSprite::Setup;
    auto Constructor0 = &FakeFDynamicSprite::Constructor0;
  }
/////////////////////////////////////////////////////
}

void InstallFFDynamicSpriteHacks()
{
  using namespace Hacks;

#if 1
  if (!FDynamicSpriteHacksInstalled)
  {
    FDynamicSpriteHacksInstalled = true;
    auto base = reinterpret_cast<uint32_t>(GetModuleHandle(L"Render.dll"));
    FDynamicSpriteFuncs::Setup = (void*)*reinterpret_cast<uint32_t*>(base + 0x10e6);
    FDynamicSpriteFuncs::Constructor0 = (void*)*reinterpret_cast<uint32_t*>(base + 0x117c);

    FDynamicSpriteDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FDynamicSpriteFuncs::Setup, *(uint64_t*)&FDynamicSpriteOverrides::Setup, &FDynamicSpriteFuncs::Setup.func64));
    FDynamicSpriteDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FDynamicSpriteFuncs::Constructor0, *(uint64_t*)&FDynamicSpriteOverrides::Constructor0, &FDynamicSpriteFuncs::Constructor0.func64));
    for (auto& detour : FDynamicSpriteDetours)
    {
      detour->hook();
    }
  }
#endif
}

void UninstallFFDynamicSpriteHacks()
{
  using namespace Hacks;

  if (FDynamicSpriteHacksInstalled)
  {
    FDynamicSpriteHacksInstalled = false;
    for (auto& detour : FDynamicSpriteDetours)
    {
      detour->unHook();
    }
    FDynamicSpriteFuncs::Setup.Restore();
    FDynamicSpriteFuncs::Constructor0.Restore();
  }
}