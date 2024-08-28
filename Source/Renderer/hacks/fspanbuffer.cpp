#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "misc.h"
#include "hacks.h"
#include "utils/debugmenu.h"

#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool FSpanBufferHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> FSpanBufferDetours;
  namespace FSpanBufferFuncs
  {
    HookableFunction CopyFromRaster = &FSpanBuffer::CopyFromRaster;
    HookableFunction CopyFromRasterUpdate = &FSpanBuffer::CopyFromRasterUpdate;
    HookableFunction BoxIsVisible = &FSpanBuffer::BoxIsVisible;
    HookableFunction MergeWith = &FSpanBuffer::MergeWith;
  }

  class FSpanBufferOverride
  {
  public:
    INT CopyFromRaster(FSpanBuffer& Screen, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster);
    int CopyFromRasterUpdate(FSpanBuffer& Screen, int RasterStartY, int RasterEndY, FRasterSpan* Raster);
    void MergeWith(const FSpanBuffer& Other);
    INT BoxIsVisible(INT X1, INT Y1, INT X2, INT Y2);
  };

  static std::unordered_set<uint32_t> m_ProcessedSurfs;
#if 0
  {
    /*
    * TODO
    *
    * Drive this whole thing with overrides in the frame context
    * On level change, we force the entire level to render, cache all static geometry.
    * Then, we return to normal culling, but still render the cached static geometry (and anything else we didn't pick up)
    */
  }
#endif

  INT FSpanBufferOverride::CopyFromRaster(FSpanBuffer& Screen, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster)
  {
    auto ctx = g_ContextManager.GetContext();
    if (ctx->overrides.bypassSpanBufferRasterization && !ctx->frameIsRasterized)
    {
      auto pThis = reinterpret_cast<FSpanBuffer*>(this);
      if (pThis->ValidLines == 0)
      {
        pThis->AllocIndexForScreen(ctx->frameSceneNode->X, 1, pThis->Mem);
        return 1;
      }

      assert(false); //unexpected to land here. Isn't this always being fed new buffers?
      auto ret = (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRaster)(Screen, RasterStartY, RasterEndY, Raster);
      return ret;
    }
    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRaster)(Screen, RasterStartY, RasterEndY, Raster);

#if 0
    uint32_t** ebpValue;
    __asm {
      mov ebpValue, ebp
    }
    uint32_t* retAddress = *(ebpValue + 1);

    //Only override things if this function is called from the OccludeBSP function.
    if (Misc::IsInDLL<Misc::DeusExDLLs::Render>(retAddress, 0x18315, 0x18337))
    {
      FBspNode* processedBspNode = *(FBspNode**)((*(uint32_t*)ebpValue) - 0x1c);
      auto ctx = g_ContextManager.GetContext();
      auto Model = ctx->frameSceneNode->Level->Model;
      auto& GSurfs = Model->Surfs;

      if ((GSurfs(processedBspNode->iSurf).PolyFlags & PF_Portal) != 0)
      {
        return 0;
      }

      float nopRange = 300.0f;
      g_DebugMenu.DebugVar("Culling SpanBuffer", "NOP-2-CopyFromRaster-Range", DebugMenuUniqueID(), nopRange, { DebugMenuValueOptions::editor::slider, 0.0f, 10000.0f });
      if (Misc::rangeFromPlayer(processedBspNode) < nopRange)
      {
        bool noOp = false;
        g_DebugMenu.DebugVar("Culling SpanBuffer", "NOP-2-CopyFromRaster", DebugMenuUniqueID(), noOp);
        if (noOp)
        {
          auto pThis = reinterpret_cast<FSpanBuffer*>(this);
          if (pThis->ValidLines != 1)
          {
            (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRaster)(Screen, 1, 2, Raster);
          }
          pThis->StartY = 1;
          pThis->EndY = 1;
          pThis->ValidLines = 1;
          return 1;
        }
      }
    }
    else
    {
      int x = 1;
    }
    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRaster)(Screen, RasterStartY, RasterEndY, Raster);
#endif
  }


  int FSpanBufferOverride::CopyFromRasterUpdate(FSpanBuffer& Screen, int RasterStartY, int RasterEndY, FRasterSpan* Raster)
  {
#if 1
    auto ctx = g_ContextManager.GetContext();

    if (ctx->overrides.bypassSpanBufferRasterization && !ctx->frameIsRasterized)
    {
      auto pThis = reinterpret_cast<FSpanBuffer*>(this);

      if (pThis->ValidLines == 0)
      { //We need to always have one valid (writable) line, otherwise we get culled
        pThis->AllocIndexForScreen(ctx->frameSceneNode->X, 1, pThis->Mem);
        return 1;
      }
      assert(false); //Isn't this always being called with validLines=0?
    }

    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRasterUpdate)(Screen, RasterStartY, RasterEndY, Raster);
#if 0
    int startY = Min(pThis->StartY, Screen.StartY);
    int endY = Max(pThis->EndY, Screen.EndY);
    int height = endY - startY;

    int thisWidth = ((pThis->Index != nullptr) ? pThis->Index[pThis->StartY]->End : 0);
    int otherWidth = ((Screen.Index != nullptr) ? Screen.Index[Screen.StartY]->End : 0);
    int width = Max(thisWidth, otherWidth);
    if (width == 0)
    {
      width = ctx->frameSceneNode->Viewport->SizeX;
    }

    auto mem = ((pThis->Mem != nullptr) ? pThis->Mem : Screen.Mem);
    pThis->AllocIndexForScreen(width, height, (mem != nullptr ? mem : &GSceneMem));
#endif

    return 1;
#if 0
    uint32_t** ebpValue;
    __asm {
      mov ebpValue, ebp
    }
    uint32_t* retAddress = *(ebpValue + 1);

    //Only override things if this function is called from the OccludeBSP function.
    if (Misc::IsInDLL<Misc::DeusExDLLs::Render>(retAddress, 0x18315, 0x1833C))
    {
      FBspNode* processedBspNode = *(FBspNode**)((*(uint32_t*)ebpValue) - 0x1c);
      auto ctx = g_ContextManager.GetContext();
      auto Model = ctx->frameSceneNode->Level->Model;
      auto& GSurfs = Model->Surfs;

      if ((GSurfs(processedBspNode->iSurf).PolyFlags & PF_Portal) != 0)
      {
        auto pThis = reinterpret_cast<FSpanBuffer*>(this);
        pThis->AllocIndexForScreen(ctx->frameSceneNode->Viewport->SizeX, ctx->frameSceneNode->Viewport->SizeY, &GSceneMem);
        return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRasterUpdate)(Screen, RasterStartY, RasterEndY, Raster);
      }

      float nopRange = 1000.0f;
      g_DebugMenu.DebugVar("Culling SpanBuffer", "NOP-1-CopyFromRasterUpdate-Range", DebugMenuUniqueID(), nopRange, { DebugMenuValueOptions::editor::slider, 0.0f, 10000.0f });
      if (Misc::rangeFromPlayer(processedBspNode) < nopRange)
      {
        bool noOp = true;
        g_DebugMenu.DebugVar("Culling SpanBuffer", "NOP-1-CopyFromRasterUpdate", DebugMenuUniqueID(), noOp);
        if (noOp)
        {
          if (!m_ProcessedSurfs.insert(processedBspNode->iSurf).second)
          {
            return 1;
          }

          auto pThis = reinterpret_cast<FSpanBuffer*>(this);
          if (pThis->ValidLines == 0)
          { //We need to always have one valid (writable) line, otherwise we get culled
            auto ctx = g_ContextManager.GetContext();
            pThis->AllocIndexForScreen( /*ctx->frameSceneNode->Viewport->SizeX*/1, /*ctx->frameSceneNode->Viewport->SizeY*/1, &GSceneMem);
          }
          return 1;
        }
      }
    }
    else
    {
      int x = 1;
    }
    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::CopyFromRasterUpdate)(Screen, RasterStartY, RasterEndY, Raster);
#endif
#endif
  }

  void FSpanBufferOverride::MergeWith(const FSpanBuffer& Other)
  {
    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::MergeWith)(Other);
#if 0
    auto pThis = reinterpret_cast<FSpanBuffer*>(this);
    auto pOther = const_cast<FSpanBuffer*>(&Other);
    if (Other.StartY<pThis->StartY || Other.EndY>pThis->EndY)
    { //We need to always have one valid (writable) line, otherwise we get culled
      auto ctx = g_ContextManager.GetContext();

      int startY = Min(pThis->StartY, Other.StartY);
      int endY = Max(pThis->EndY, Other.EndY);
      int height = endY - startY;

      SWORD thisStart, thisEnd;
      SWORD otherStart, otherEnd;
      pThis->GetValidRange(&thisStart, &thisEnd);
      pOther->GetValidRange(&otherStart, &otherEnd);
      int thisWidth = ((pThis->EndY > pThis->StartY && pThis->Index != nullptr) ? pThis->Index[thisStart]->End : 0);
      int otherWidth = ((Other.EndY > Other.StartY && Other.Index != nullptr) ? Other.Index[otherStart]->End : 0);
      int width = Max(thisWidth, otherWidth);
      if (width == 0)
      {
        width = ctx->frameSceneNode->Viewport->SizeX;
      }

      auto mem = ((pThis->Mem != nullptr) ? pThis->Mem : Other.Mem);
      pThis->AllocIndexForScreen(width, height, (mem != nullptr ? mem : &GSceneMem));
    }
#endif
#if 0
    uint32_t** ebpValue;
    __asm {
      mov ebpValue, ebp
    }
    uint32_t* retAddress = *(ebpValue + 1);
    //
    if (Misc::IsInDLL<Misc::DeusExDLLs::Render>(retAddress, 0x19394, 0x193a6))
    {
      bool noOp = true;
      g_DebugMenu.DebugVar("Culling SpanBuffer", "NOP-0-MergeWith", DebugMenuUniqueID(), noOp);
      if (noOp)
      {
        //FBspNode* processedBspNode = *(FBspNode**)((*(uint32_t*)ebpValue)-0x1c);
        auto pThis = reinterpret_cast<FSpanBuffer*>(this);
        if (pThis->ValidLines == 0)
        { //We need to always have one valid (writable) line, otherwise we get culled
          auto ctx = g_ContextManager.GetContext();
          pThis->AllocIndexForScreen(ctx->frameSceneNode->Viewport->SizeX, ctx->frameSceneNode->Viewport->SizeY, &GSceneMem);
        }
        return;
      }
    }
    if (Other.ValidLines == 0)
    {
      int x = 1;
    }

    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::MergeWith)(Other);
#endif
  }

  INT FSpanBufferOverride::BoxIsVisible(INT X1, INT Y1, INT X2, INT Y2)
  {
    auto ctx = g_ContextManager.GetContext();
    if (ctx->overrides.bypassSpanBufferRasterization && !ctx->frameIsRasterized)
    {
      return 1;
    }

    return (reinterpret_cast<FSpanBuffer*>(this)->*FSpanBufferFuncs::BoxIsVisible)(X1, Y1, X2, Y2);
  };

  namespace FSpanBufferOverrides
  {
    INT(FSpanBufferOverride::* CopyFromRaster)(FSpanBuffer& ScreenSpanBuffer, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster) = &FSpanBufferOverride::CopyFromRaster;
    INT(FSpanBufferOverride::* CopyFromRasterUpdate)(FSpanBuffer& ScreenSpanBuffer, INT RasterStartY, INT RasterEndY, FRasterSpan* Raster) = &FSpanBufferOverride::CopyFromRasterUpdate;
    void(FSpanBufferOverride::* MergeWith)(const FSpanBuffer& Other) = &FSpanBufferOverride::MergeWith;
    INT(FSpanBufferOverride::* BoxIsVisible)(INT X1, INT Y1, INT X2, INT Y2) = &FSpanBufferOverride::BoxIsVisible;
  }
}

void InstallFSpanBufferHacks()
{
  using namespace Hacks;

  if (!FSpanBufferHacksInstalled)
  {
    FSpanBufferHacksInstalled = true;
    FSpanBufferDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSpanBufferFuncs::CopyFromRaster, *(uint64_t*)&FSpanBufferOverrides::CopyFromRaster, &FSpanBufferFuncs::CopyFromRaster.func64));
    FSpanBufferDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSpanBufferFuncs::CopyFromRasterUpdate, *(uint64_t*)&FSpanBufferOverrides::CopyFromRasterUpdate, &FSpanBufferFuncs::CopyFromRasterUpdate.func64));
    FSpanBufferDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSpanBufferFuncs::MergeWith, *(uint64_t*)&FSpanBufferOverrides::MergeWith, &FSpanBufferFuncs::MergeWith.func64));
    FSpanBufferDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&FSpanBufferFuncs::BoxIsVisible, *(uint64_t*)&FSpanBufferOverrides::BoxIsVisible, &FSpanBufferFuncs::BoxIsVisible.func64));
    for (auto& detour : FSpanBufferDetours)
    {
      auto result = detour->hook();
      assert(result);
    }
  }
}

void UninstallFSpanBufferHacks()
{
  using namespace Hacks;

  if (FSpanBufferHacksInstalled)
  {
    FSpanBufferHacksInstalled = false;
    for (auto& detour : FSpanBufferDetours)
    {
      detour->unHook();
    }
    FSpanBufferFuncs::CopyFromRaster.Restore();
    FSpanBufferFuncs::CopyFromRasterUpdate.Restore();
    FSpanBufferFuncs::BoxIsVisible.Restore();
    FSpanBufferFuncs::MergeWith.Restore();
    FSpanBufferDetours.clear();
  }
}