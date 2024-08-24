#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "misc.h"
#include "hacks.h"
#include "utils/utils.h"
#include "utils/configmanager.h"
#include "utils/debugmenu.h"
#include "rendering/scenemanager.h"
#include "uefacade.h"

#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace Hacks
{
  bool URenderHacksInstalled = false;
  std::vector<std::shared_ptr<PLH::IHook>> URenderDetours;
  namespace URenderFuncs
  {
    HookableFunction OccludeBsp = &URender::OccludeBsp;
    HookableFunction DrawFrame = &URender::DrawFrame;
    HookableFunction OccludeFrame = &URender::OccludeFrame;
    HookableFunction ClipBspSurf = &URender::ClipBspSurf;
    HookableFunction DrawMesh = &URender::DrawMesh;
    HookableFunction SetupDynamics = &URender::SetupDynamics;
    UBOOL(*SetupRaster)(FTransform** Pts, INT NumPts, FSpanBuffer* Span, INT EndY) = nullptr; /*not exported*/
  }
  namespace URenderVTableFuncs
  {
    FSceneNode* (__thiscall* CreateMasterFrame)(URender* pThis, UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds) = nullptr;
    UBOOL(__thiscall* BoundVisible)(URender* pThis, FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Result) = nullptr;
    void(__thiscall* PreRender)(URender*, FSceneNode*) = nullptr;
    void(__thiscall* PostRender)(URender*, FSceneNode*) = nullptr;
    void(__thiscall* DrawWorld)(URender*, FSceneNode*) = nullptr;
  }

  class URenderOverride
  {
    /*
    * PreRender
    *   DrawWorld
    *     OccludeFrame
    *         OccludeBsp
    *              OccludeFrame (recurse)
    *     DrawFrame
    *         DrawFrame (recurse)
    *             Device->DrawComplexShapes
    * PostRender
    */
  public:
    /*virtual*/ void PreRender(FSceneNode* Frame)
    {
      Misc::g_Facade->GetHLRenderer()->OnRenderingBegin(Frame);
      URenderVTableFuncs::PreRender(GRender, Frame);
    };
    /*virtual*/ void PostRender(FSceneNode* Frame)
    {
      URenderVTableFuncs::PostRender(GRender, Frame);
      Misc::g_Facade->GetHLRenderer()->OnRenderingEnd(Frame);
    };
    /*virtual*/ void DrawWorld(FSceneNode* Frame)
    {
      for (auto iActor = 0; iActor < Frame->Level->Actors.Num(); iActor++)
      {
        auto actor = Frame->Level->Actors(iActor);
        if (actor != nullptr)
        {
          Frame->Level->Actors(iActor)->VisibilityRadius = FLT_MAX;
          Frame->Level->Actors(iActor)->VisibilityHeight = FLT_MAX;
        }
      }

      URenderVTableFuncs::DrawWorld(GRender, Frame);
    };
    /*virtual*/ FSceneNode* CreateMasterFrame(UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds)
    {
      g_DebugMenu.DebugVar("Global", "FOV Angle", DebugMenuUniqueID(), Viewport->Actor->FovAngle, { DebugMenuValueOptions::editor::slider, 0.0f, 180.0f });
      g_DebugMenu.DebugVar("Global", "Frame X", DebugMenuUniqueID(), Viewport->SizeX, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, -8000, 8000 });
      g_DebugMenu.DebugVar("Global", "Frame Y", DebugMenuUniqueID(), Viewport->SizeY, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 1, 2880 });
      g_DebugMenu.DebugVar("Global", "Orthozoom", DebugMenuUniqueID(), Viewport->Actor->OrthoZoom, { DebugMenuValueOptions::editor::slider, -1000000.0f, 1000000.0f });

      FVector LocationOffset{ 0.0f,0.0f,0.0f };
      g_DebugMenu.DebugVar("Global", "Frame Position Offset", DebugMenuUniqueID(), LocationOffset);
      FSceneNode* Frame = URenderVTableFuncs::CreateMasterFrame(GRender, Viewport, Location + LocationOffset, Rotation, Bounds);
      return Frame;
    }

    /*virtual*/ UBOOL BoundVisible(FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Result)
    {
      auto ret = URenderVTableFuncs::BoundVisible(GRender, Frame, Bound, SpanBuffer, Result);
      g_DebugMenu.DebugVar("Culling", "BoundVisible result", DebugMenuUniqueID(), ret);
      bool forceFrame = false;
      g_DebugMenu.DebugVar("Culling", "BoundVisible result, force frame", DebugMenuUniqueID(), forceFrame);
      if (ret == 0 && forceFrame)
      {
        Result.MinX = 0;
        Result.MinY = 0;
        Result.MaxX = Frame->FX;
        Result.MaxY = Frame->FY;
        Result.MinZ = 0;
        return 1;
      }
      return ret;
    }

    void OccludeBsp(FSceneNode* Frame);
    void OccludeFrame(FSceneNode* Frame);
    void DrawMesh(FSceneNode* Frame, AActor* Owner, AActor* LightSink, FSpanBuffer* SpanBuffer, AZoneInfo* Zone, const FCoords& Coords, FVolActorLink* LeafLights, FActorLink* Volumetrics, DWORD PolyFlags);
    void SetupDynamics(FSceneNode* Frame, AActor* Exclude);
    void ComputeRenderSize(FSceneNode* Frame);
    void DrawFrame(FSceneNode* Frame);
    INT ClipBspSurf(INT iNode, FTransform**& Result);

    static UBOOL SetupRaster(FTransform** Pts, INT NumPts, FSpanBuffer* Span, INT EndY);
  };

  void URenderOverride::OccludeBsp(FSceneNode* Frame)
  {
    //if (Frame->Parent != nullptr)
    //{
    //  return;
    //}
    if (Frame->Mirror < 0.0f)
    {
      return;
    }

    FrameContextManager::ScopedContext ctx;
    ctx->frameSceneNode = Frame;
    (GRender->*URenderFuncs::OccludeBsp)(Frame);
  }

  void URenderOverride::OccludeFrame(FSceneNode* Frame)
  {
    auto ctx = g_ContextManager.GetContext();
    Frame->Viewport->ExtraPolyFlags |= PF_NoMerge;

    bool markTwoSided = false;
    {
      g_DebugMenu.DebugVar("Culling", "Flag all geo as twosided", DebugMenuUniqueID(), markTwoSided, {});
      if (markTwoSided)
      {
        Frame->Viewport->ExtraPolyFlags |= PF_TwoSided;
      }
    }

    bool markForceViewZone = false;
    {
      g_DebugMenu.DebugVar("Culling", "Force viewzone", DebugMenuUniqueID(), markForceViewZone, {});
      if (markForceViewZone)
      {
        Frame->Viewport->ExtraPolyFlags |= PF_ForceViewZone;
      }
    }

    bool applyNoBoundRejection = true;
    {
      g_DebugMenu.DebugVar("Culling", "No bound rejection", DebugMenuUniqueID(), applyNoBoundRejection, {});
      if (applyNoBoundRejection)
      {
        Frame->Viewport->ExtraPolyFlags |= PF_NoBoundRejection;
      }
    }

    if (g_options.cameraTest)
    {
#if 0
      Frame->Coords *= FRotator(sinf(appSeconds()) * 65536.0f * 0.01f, 0, 0);
      Frame->Coords *= (FVector(sinf(appSeconds() * 2.0f), cosf(appSeconds() * 2.0f), 0.0f) * 50.0f);
#endif
    }

    FSceneNode origFrame = *Frame;
    FCoords origCoords = origFrame.Coords;
    FVector origPos = origCoords.Origin;
    FRotator origRot = Frame->Viewport->Actor->ViewRotation;
    float origFovAngle = Frame->Viewport->Actor->FovAngle;


    ADeusExPlayer* player = CastChecked<ADeusExPlayer>(Frame->Viewport->Actor);
    const bool isInCutscene = (player->ConPlay != nullptr && player->bBehindView == 1);

    g_DebugMenu.DebugVar("Culling", "Backwards Occlusion Enabled", DebugMenuUniqueID(), g_options.backwardsOcclusionPass);
    if (g_options.backwardsOcclusionPass && /*Frame->Parent == nullptr &&*/ !isInCutscene)
    {
      const FColor angleColors[] = { FColor(255,0,0), FColor(0,255,0), FColor(0,0,255) };
      //constexpr float angles[] = {0.66f, 0.33f, 0.0f};
      constexpr float angles[] = { 0.50f, 0.0f };
      //constexpr float angles[] = {0.0f};

      auto originalSpan = Frame->Span;
      auto originalBrushTracker = Frame->Level->BrushTracker;
      //Frame->Level->BrushTracker = nullptr;
      static std::vector<std::tuple<FVector, FVector, FColor>> lines;
      bool drawLines = true;
      g_DebugMenu.DebugVar("Debug", "Update culling frustrum lines", DebugMenuUniqueID(), drawLines);
      if (drawLines)
      {
        lines.clear();
      }

      for (int i = 0; i < std::size(angles); i++)
      {
        const bool isLast = ((i + 1) == std::size(angles));
        const float& angle = angles[i];
        FrameContextManager::ScopedContext ctx;
        {
          ctx->frameSceneNode = Frame;
          ctx->overrides.skipDynamicFiltering = false; //keep off; otherwise meshes are culled from reflections as well.
          ctx->overrides.bypassSetupDynamics =  false; //!isLast; //gives a perf boost when disabled, but we lose reflections of actors behind us.

          float fov = 155.0f;
          g_DebugMenu.DebugVar("Culling", "Backwards Occlusion FOV", DebugMenuUniqueID(), fov, { DebugMenuValueOptions::editor::slider, 0.0f, 179.999f });

          Frame->Viewport->Actor->FovAngle = fov;
          if (!isLast)
          {
            Frame->Span = New<FSpanBuffer>(GSceneMem);
            Frame->Span->AllocIndexForScreen(Frame->Viewport->SizeX, Frame->Viewport->SizeY, &GSceneMem);
          }
          else
          {
            Frame->Span = originalSpan;
          }

          float backwardsAdjustment = 100.0f;
          g_DebugMenu.DebugVar("Culling", "Backwards Occlusion Adjustment", DebugMenuUniqueID(), backwardsAdjustment, { DebugMenuValueOptions::editor::slider, -100.0f, 1000.0f });
          auto newRotation = origRot + FRotator(0.0f, 65536.0f * angle, 0.0f);

          auto newPosition = origPos;
          if (!isLast)
          {
            newRotation.Pitch = 0;
            newPosition -= (newRotation.Vector() * backwardsAdjustment);
          }
          Frame->ComputeRenderCoords(newPosition, newRotation);

#if 0
          //TODO: get a node id and track it through the system to see where it gets culled
          FLOAT TempSigns[2] = { -1.0,+1.0 };
          for (INT i = 0; i < 2; i++)
          {
            for (INT j = 0; j < 2; j++)
            {
              Frame->ViewSides[i * 2 + j] = FVector(TempSigns[i] * Frame->FX2, TempSigns[j] * Frame->FY2, 0.0f).UnsafeNormal().TransformVectorBy(Frame->Uncoords);
            }
            Frame->ViewPlanes[i] = FPlane
            (
              Frame->Coords.Origin,
              FVector(0, TempSigns[i] / Frame->FY2, 0.0f).UnsafeNormal().TransformVectorBy(Frame->Uncoords)
            );
            Frame->ViewPlanes[i + 2] = FPlane
            (
              Frame->Coords.Origin,
              FVector(TempSigns[i] / Frame->FX2, 0, 0.0f).UnsafeNormal().TransformVectorBy(Frame->Uncoords)
            );
          }
          Frame->PrjXM = (0 - Frame->FX2) * (-Frame->RProj.Z);
          Frame->PrjXP = (Frame->FX - Frame->FX2) * (+Frame->RProj.Z);
          Frame->PrjYM = (0 - Frame->FY2) * (-Frame->RProj.Z);
          Frame->PrjYP = (Frame->FY - Frame->FY2) * (+Frame->RProj.Z);
#endif
          if (drawLines)
          {
            lines.push_back({ Frame->Coords.Origin, Frame->Coords.Origin + newRotation.Vector() * 100.0f, angleColors[i] });
            lines.push_back({ Frame->Coords.Origin, Frame->Coords.Origin + (Frame->ViewSides[0] * 1000.0f), angleColors[i] });
            lines.push_back({ Frame->Coords.Origin, Frame->Coords.Origin + (Frame->ViewSides[1] * 1000.0f), angleColors[i] });
            lines.push_back({ Frame->Coords.Origin, Frame->Coords.Origin + (Frame->ViewSides[2] * 1000.0f), angleColors[i] });
            lines.push_back({ Frame->Coords.Origin, Frame->Coords.Origin + (Frame->ViewSides[3] * 1000.0f), angleColors[i] });
          }
          (GRender->*URenderFuncs::OccludeFrame)(Frame);
        }
      }

      Frame->Level->BrushTracker = originalBrushTracker;
      Frame->Span = originalSpan;
      Frame->Viewport->Actor->FovAngle = origFovAngle;
      Frame->ComputeRenderCoords(origPos, origRot);
      //for (auto& l : lines)
      //{
      //  ::Misc::g_Facade->GetHLRenderer()->Draw3DLine(Frame, std::get<0>(l), std::get<1>(l), std::get<2>(l));
      //}
    }
    else
    {
      FrameContextManager::ScopedContext ctx;
      ctx->frameSceneNode = Frame;
      (GRender->*URenderFuncs::OccludeFrame)(Frame);
    }
  }

  void URenderOverride::ComputeRenderSize(FSceneNode* Frame)
  {
    float fovangle = Frame->Viewport->Actor->FovAngle;
    Frame->FX = (FLOAT)Frame->X;
    Frame->FY = (FLOAT)Frame->Y;
    Frame->FX2 = Frame->FX * 0.5;
    Frame->FY2 = Frame->FY * 0.5;
    Frame->FX15 = (Frame->FX + 1.0001) * 0.5;
    Frame->FY15 = (Frame->FY + 1.0001) * 0.5;
    Frame->Proj = FVector(0.5 - 0.5 * Frame->FX, 0.5 - 0.5 * Frame->FY, 0.5 * Frame->FX / appTan(fovangle * PI / 360.0));
    Frame->RProj = FVector(1 / Frame->Proj.X, 1 / Frame->Proj.Y, 1 / Frame->Proj.Z);
    Frame->Zoom = Frame->Viewport->Actor->OrthoZoom / (Frame->FX * 15.0);
    Frame->PrjXM = (0 - Frame->FX2) * (-Frame->RProj.Z);
    Frame->PrjXP = (Frame->FX - Frame->FX2) * (+Frame->RProj.Z);
    Frame->PrjYM = (0 - Frame->FY2) * (-Frame->RProj.Z);
    Frame->PrjYP = (Frame->FY - Frame->FY2) * (+Frame->RProj.Z);


    // Precompute side info.
    FLOAT TempSigns[2] = { -1.0,+1.0 };
    for (INT i = 0; i < 2; i++)
    {
      for (INT j = 0; j < 2; j++)
      {
        Frame->ViewSides[i * 2 + j] = FVector(TempSigns[i] * Frame->FX2, TempSigns[j] * Frame->FY2, Frame->Proj.Z).UnsafeNormal().TransformVectorBy(Frame->Uncoords);
      }
      Frame->ViewPlanes[i] = FPlane
      (
        Frame->Coords.Origin,
        FVector(0, TempSigns[i] / Frame->FY2, 1.0 / Frame->Proj.Z).UnsafeNormal().TransformVectorBy(Frame->Uncoords)
      );
      Frame->ViewPlanes[i + 2] = FPlane
      (
        Frame->Coords.Origin,
        FVector(TempSigns[i] / Frame->FX2, 0, 1.0 / Frame->Proj.Z).UnsafeNormal().TransformVectorBy(Frame->Uncoords)
      );
    }

  }

  void URenderOverride::DrawFrame(FSceneNode* Frame)
  {
    //This function is called recursively, so while it looks like we can do symmetric
    //enter/exit style functions, we can't. So, instead, we just register with the
    //scene manager. 
    FrameContextManager::ScopedContext ctx;
    ctx->frameSceneNode = Frame;
    auto zoneIndex = Frame->ZoneNumber;
    bool frameIsSkybox = false;
    if (zoneIndex >= 0 && zoneIndex < FBspNode::MAX_ZONES)
    {
      auto skyZone = Frame->Level->GetZoneActor(Frame->ZoneNumber)->SkyZone;
      frameIsSkybox = (skyZone != nullptr) && (skyZone->Region.ZoneNumber == zoneIndex);
      ctx->frameIsSkybox = frameIsSkybox;
    }

    if (Frame->Parent == nullptr || frameIsSkybox)
    {
      g_SceneManager.PushScene(Frame);
      (GRender->*URenderFuncs::DrawFrame)(Frame);
      g_SceneManager.PopScene(Frame);
    }

#if 0
    if (g_options.cameraTest)
    {
#if 0
      Frame->Coords *= FRotator(sinf(appSeconds()) * 65536.0f * 0.01f, 0, 0);
      Frame->Coords *= (FVector(sinf(appSeconds() * 2.0f), cosf(appSeconds() * 2.0f), 0.0f) * 50.0f);
      //Frame->ComputeRenderCoords(Frame->Coords.Origin, Frame->Coords.OrthoRotation());
      FakeComputeRenderSize(Frame);
#endif
    }
#endif

  }

  INT URenderOverride::ClipBspSurf(INT iNode, FTransform**& Result)
  {
    auto ctx = g_ContextManager.GetContext();
    auto Model = ctx->frameSceneNode->Level->Model;
    auto& GSurfs = Model->Surfs;
    auto& GNodes = Model->Nodes;
    auto& GVerts = Model->Verts;
    auto& GVertPoints = Model->Points;
    auto* GFrame = ctx->frameSceneNode;
    auto* GPoints = &Model->Points(0);

    bool overrideClipBsp = false;
    g_DebugMenu.DebugVar("Culling", "Override ClipBSP", DebugMenuUniqueID(), overrideClipBsp, {});
    if (!overrideClipBsp)
    {
      return (GRender->*URenderFuncs::ClipBspSurf)(iNode, Result);
    }

    static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
    FLOAT Dot[FBspNode::MAX_FINAL_VERTICES];
    auto Pipe = [](FTransform& Result, const FSceneNode* Frame, const FVector& InVector) {
      static FLOAT Half = 0.5;
      static FLOAT ClipXM, ClipXP, ClipYM, ClipYP;
      static const BYTE OutXMinTab[2] = { 0, FVF_OutXMin };
      static const BYTE OutXMaxTab[2] = { 0, FVF_OutXMax };
      static const BYTE OutYMinTab[2] = { 0, FVF_OutYMin };
      static const BYTE OutYMaxTab[2] = { 0, FVF_OutYMax };
      Result.Point = InVector.TransformPointBy(Frame->Coords);

      ClipXM = Frame->PrjXM * Result.Point.Z + Result.Point.X;
      ClipXP = Frame->PrjXP * Result.Point.Z - Result.Point.X;
      ClipYM = Frame->PrjYM * Result.Point.Z + Result.Point.Y;
      ClipYP = Frame->PrjYP * Result.Point.Z - Result.Point.Y;

      Result.Flags =
        (OutXMinTab[ClipXM < 0.0]
          + OutXMaxTab[ClipXP < 0.0]
          + OutYMinTab[ClipYM < 0.0]
          + OutYMaxTab[ClipYP < 0.0]);

      bool forceNoClip = false;
      g_DebugMenu.DebugVar("Culling", "Force ClipBSP off", DebugMenuUniqueID(), forceNoClip, {});
      if (forceNoClip)
      {
        Result.Flags = 0;
      }

      if (!Result.Flags)
      {
        Result.RZ = Frame->Proj.Z / Result.Point.Z;
        Result.ScreenX = std::clamp<float>((Result.Point.X * Result.RZ + Frame->FX15), 0, Frame->FX);
        Result.ScreenY = std::clamp<float>((Result.Point.Y * Result.RZ + Frame->FY15), 0, Frame->FY);
        Result.IntY = std::clamp<INT>(appFloor(Result.ScreenY), 0, Frame->Y);
      }
    };

    auto Clip = [&Dot, &GFrame](FTransform** Dest, FTransform** Src, INT SrcNum) {
      INT DestNum = 0;
      for (INT i = 0, j = SrcNum - 1; i < SrcNum; j = i++)
      {
        if (*(INT*)(Dot + j) >= 0)
        {
          Dest[DestNum++] = Src[j];
        }
        if ((*(INT*)(Dot + j) ^ *(INT*)(Dot + i)) < 0)
        {
          FTransform* T = Dest[DestNum++] = New<FTransform>(GSceneMem);
          *T = *Src[j];
        }
#if 0
        if (*(INT*)(Dot + j) >= 0)
        {
          Dest[DestNum++] = Src[j];
        }
#if 1
        if ((*(INT*)(Dot + j) ^ *(INT*)(Dot + i)) < 0)
        {
          FTransform* T = Dest[DestNum++] = New<FTransform>(GSceneMem);
          FLOAT Alpha = Dot[j] / (Dot[j] - Dot[i]);
          T->Point.X = Src[j]->Point.X + (Src[i]->Point.X - Src[j]->Point.X) * Alpha;
          T->Point.Y = Src[j]->Point.Y + (Src[i]->Point.Y - Src[j]->Point.Y) * Alpha;
          T->Point.Z = Src[j]->Point.Z + (Src[i]->Point.Z - Src[j]->Point.Z) * Alpha;
          T->Project(GFrame);
        }
#else
        if ((*(INT*)(Dot + j) ^ *(INT*)(Dot + i)) < 0)
        {
          FTransform* T = Dest[DestNum++] = New<FTransform>(GSceneMem);
          *T = *Src[j];
        }
#endif
#endif
      }
      return DestNum;
    };

    // Transform.
    FBspNode* Node = &GNodes(iNode);
    INT       NumPts = Node->NumVertices;
    FVert* VertPool = &GVerts(Node->iVertPool);
    BYTE      Outcode = FVF_OutReject;
    BYTE      AllCodes = 0;
    for (INT i = 0; i < NumPts; i++)
    {
      INT pPoint = VertPool[i].pVertex;
      URender::FStampedPoint& S = URender::PointCache[pPoint];
      if (S.Stamp != URender::Stamp)
      {
        S.Stamp = URender::Stamp;
        S.Point = new(URender::VectorMem)FTransform;
        Pipe(*S.Point, GFrame, GPoints[pPoint]);
      }
      LocalPts[i] = S.Point;
      BYTE Flags = S.Point->Flags;
      Outcode &= Flags;
      AllCodes |= Flags;
    }
    //if( Outcode )
    //  return 0;

    // Clip.
    FTransform** Pts = LocalPts;
    if (AllCodes)
    {
      if (AllCodes & FVF_OutXMin)
      {
        static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
        for (INT i = 0; i < NumPts; i++)
          Dot[i] = GFrame->PrjXM * Pts[i]->Point.Z + Pts[i]->Point.X;
        NumPts = Clip(LocalPts, Pts, NumPts);
        if (!NumPts)
          return 0;
        Pts = LocalPts;
      }
      if (AllCodes & FVF_OutXMax)
      {
        static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
        for (INT i = 0; i < NumPts; i++)
          Dot[i] = GFrame->PrjXP * Pts[i]->Point.Z - Pts[i]->Point.X;
        NumPts = Clip(LocalPts, Pts, NumPts);
        if (!NumPts)
          return 0;
        Pts = LocalPts;
      }
      if (AllCodes & FVF_OutYMin)
      {
        static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
        for (INT i = 0; i < NumPts; i++)
          Dot[i] = GFrame->PrjYM * Pts[i]->Point.Z + Pts[i]->Point.Y;
        NumPts = Clip(LocalPts, Pts, NumPts);
        if (!NumPts)
          return 0;
        Pts = LocalPts;
      }
      if (AllCodes & FVF_OutYMax)
      {
        static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
        for (INT i = 0; i < NumPts; i++)
          Dot[i] = GFrame->PrjYP * Pts[i]->Point.Z - Pts[i]->Point.Y;
        NumPts = Clip(LocalPts, Pts, NumPts);
        if (!NumPts)
          return 0;
        Pts = LocalPts;
      }
    }
    if (GFrame->NearClip.W != 0.0)
    {
      UBOOL Clipped = 0;
      for (INT i = 0; i < NumPts; i++)
      {
        Dot[i] = GFrame->NearClip.PlaneDot(Pts[i]->Point);
        Clipped |= (Dot[i] < 0.0);
      }
      if (Clipped)
      {
        static FTransform* LocalPts[FBspNode::MAX_FINAL_VERTICES];
        NumPts = Clip(LocalPts, Pts, NumPts);
        if (!NumPts)
          return 0;
        Pts = LocalPts;
      }
    }
    Result = Pts;
    return NumPts;
  }

  //Hijacking of a specific call to URender::SetupRaster, to force an early-out during occlusion.
  UBOOL URenderOverride::SetupRaster(FTransform** Pts, INT NumPts, FSpanBuffer* Span, INT EndY)
  {
#if 1
    //The maximum occlusion distance is set when we are running additional
    //occludeBSP passes, and is used to control the performance cost of doing additional
    //occlusion checks.
    bool hasMaxDistance = true;
    g_DebugMenu.DebugVar("Culling", "SetupRaster Use Max Distance", DebugMenuUniqueID(), hasMaxDistance);
    auto ctx = g_ContextManager.GetContext();
    if (ctx->overrides.maxOccludeBspDistance)
    {
      FVector center = FVector(0.0f, 0.0f, 0.0f);
      for (int i = 0; i < NumPts; i++)
      {
        center += Pts[i]->Point;
      }
      center /= float(NumPts);

      float d = center.Size();
      if (d > ctx->overrides.maxOccludeBspDistance)
      {
        return false;
      }
    }

    bool result = URenderFuncs::SetupRaster(Pts, NumPts, Span, EndY);
    g_DebugMenu.DebugVar("Culling", "SetupRaster result", DebugMenuUniqueID(), result);
    return result;
#else
    return OriginalSetupRasterOverride(Pts, NumPts, Span, EndY);
#endif
  }

  void URenderOverride::DrawMesh(FSceneNode* Frame, AActor* Actor, AActor* LightSink, FSpanBuffer* SpanBuffer, AZoneInfo* Zone, const FCoords& Coords, FVolActorLink* LeafLights, FActorLink* Volumetrics, DWORD PolyFlags)
  {
    /*
    * notes!
    * There is a section in UnMeshRn.cpp that says it does backface culling on meshes:
    * 	// Backface reject it.
    if( (PolyFlags & PF_TwoSided) && FTriple(Pts[0]->Point,Pts[1]->Point,Pts[2]->Point) <= 0.0 )
    {
    if( !(PolyFlags & PF_TwoSided) )
    return;
    Exchange( Pts[2], Pts[0] );
    }
    * But that seems incorrect. It's basically a way of flipping all triangles so they don't get
    * backface-culled by the device renderer later. ie, ensuring double-sided triangles are always visible.
    * The pipeline does backface culling in software earlier; earlier during DrawMesh():
    * 			// See if potentially visible.
    if( !(V1.Flags & V2.Flags & V3.Flags) )
    {
    if
    (	(PolyFlags & (PF_TwoSided|PF_Flat|PF_Invisible))!=(PF_Flat)
    ||	Frame->Mirror*(V1.Point|TriNormals[i])<0.0 )
    {
    //snip...
    }
    }
    * I can disable culling by patching DrawLodMesh:
    * >render.dll (10F30000)
    * 0000FF95: 6 bytes -> 0x90
    0000FFC9: 6 bytes -> 0x90

    00010317: 6 bytes -> 0x90
    0001034B: 6 bytes -> 0x90

    and DrawMesh:
    0000DC81 6 bytes -> 90
    0000DCBB 6 bytes -> 90
    */

    for (FDynamicSprite* Sprite = Frame->Sprite; Sprite; Sprite = Sprite->RenderNext)
    {
      if (Sprite->Actor == Frame->Viewport->Actor)
      {
        int x = 1;
      }
    }

    if (g_options.hasObjectMeshes)
    {
      static UBOOL& HasSpecialCoords = []() -> UBOOL& {
        uint32_t baseAddress = (uint32_t)GetModuleHandle(L"Render.dll");
        return *reinterpret_cast<UBOOL*>(baseAddress + 0x4eb10);
      }();
      static FCoords& SpecialCoords = []() -> FCoords& {
        uint32_t baseAddress = (uint32_t)GetModuleHandle(L"Render.dll");
        return *reinterpret_cast<FCoords*>(baseAddress + 0x4ea08);
      }();

      FrameContextManager::ScopedContext ctx;
      ctx->frameSceneNode = Frame;
      ctx->drawcallInfo.emplace();
      ctx->drawcallInfo->Owner = Actor;
      ctx->drawcallInfo->LightSink = LightSink;
      ctx->drawcallInfo->SpanBuffer = SpanBuffer;
      ctx->drawcallInfo->Zone = Zone;
      ctx->drawcallInfo->Coords = Coords;
      ctx->drawcallInfo->LeafLights = LeafLights;
      ctx->drawcallInfo->Volumetrics = Volumetrics;
      ctx->drawcallInfo->PolyFlags = PolyFlags;
      if (HasSpecialCoords)
      {
        ctx->drawcallInfo->SpecialCoords = SpecialCoords;
      }

      //For LOD meshes (which are most meshes in game), disable the lodding system
      //and always render the highest level of detail.
      const bool isLodMesh = Actor->Mesh->IsA(ULodMesh::StaticClass());
      if (isLodMesh)
      {
        ((ULodMesh*)Actor->Mesh)->LODStrength = 0.0;
        ((ULodMesh*)Actor->Mesh)->LODMorph = 1.0;
        ((ULodMesh*)Actor->Mesh)->LODMinVerts = ((ULodMesh*)Actor->Mesh)->Verts.Num();
      }

      //Store the transformation details. We'll set them to origin
      //so that UE renders the model in local/model space.
      auto origLocation = Actor->Location;
      auto origPrePivot = Actor->PrePivot;
      auto origRotation = Actor->Rotation;
      auto origFrameCoords = Frame->Coords;
      auto origUncoords = Frame->Uncoords;
      auto origCoords = Coords;

      //Render the player body mesh (with an offset)
      auto playerPawn = Frame->Viewport->Actor;
      FVector offset{ 0.0f, 0.0f, 0.0f };
      if (g_ConfigManager.GetRenderPlayerBody() && Actor == playerPawn)
      {
        float collisionRadiusMultiplier = 2.5f;
        g_DebugMenu.DebugVar("Rendering", "Player Body Offset factor", DebugMenuUniqueID(), collisionRadiusMultiplier, { DebugMenuValueOptions::editor::slider, -5.0f, 5.0f });
        offset = FVector(-playerPawn->CollisionRadius * collisionRadiusMultiplier, 0.0f, 0.0f);

        //todo, calculate offset by intersecting the body mesh with the camera frustrum.
        //currently sometimes the playermesh moves through the camera plane.
      }

      //Inform high-level renderer we're about to draw a mesh
      ::Misc::g_Facade->GetHLRenderer()->OnDrawMeshBegin(Frame, Actor);

      //Disable view transformations and move the rendered mesh to origin with no rotation.
      //We'll do the transformations in the fixed-function pipeline.
      //This way, RTX Remix can pick up the mesh, so that we can replace static meshes.
      Frame->Coords = FCoords(FVector(0.0f, 0.0f, 0.0f));
      Frame->Uncoords = FCoords(FVector(0.0f, 0.0f, 0.0f));
      Actor->Location = FVector(0.0f, 0.0f, 0.0f) + offset;
      Actor->Rotation = FRotator(0, 0, 0);
      const_cast<FCoords&>(Coords) = FCoords(FVector(0.0f, 0.0f, 0.0f));;
      (GRender->*URenderFuncs::DrawMesh)(Frame, Actor, LightSink, SpanBuffer, Zone, Coords, LeafLights, Volumetrics, PolyFlags);
      const_cast<FCoords&>(Coords) = origCoords;
      Actor->Location = origLocation;
      Actor->Rotation = origRotation;
      Frame->Coords = origFrameCoords;
      Frame->Uncoords = origUncoords;

      //Inform the high-level renderer that we're done, and can submit the vertex buffer for rendering.
      ::Misc::g_Facade->GetHLRenderer()->OnDrawMeshEnd(Frame, Actor);
    }
  }

  void URenderOverride::SetupDynamics(FSceneNode* Frame, AActor* Exclude)
  {
    auto ctx = g_ContextManager.GetContext();

    if (!ctx->overrides.bypassSetupDynamics)
    {
      (GRender->*URenderFuncs::SetupDynamics)(Frame, Exclude);
    }
  }

  namespace URenderOverrides
  {
    void(URenderOverride::* OccludeBsp)(FSceneNode* Frame) = &URenderOverride::OccludeBsp;
    void(URenderOverride::* DrawFrame)(FSceneNode* Frame) = &URenderOverride::DrawFrame;
    void(URenderOverride::* OccludeFrame)(FSceneNode* Frame) = &URenderOverride::OccludeFrame;
    void(URenderOverride::* DrawMesh)(FSceneNode* Frame, AActor* Owner, AActor* LightSink, FSpanBuffer* SpanBuffer, AZoneInfo* Zone, const FCoords& Coords, FVolActorLink* LeafLights, FActorLink* Volumetrics, DWORD PolyFlags) = &URenderOverride::DrawMesh;
    void(URenderOverride::* SetupDynamics)(FSceneNode* Frame, AActor* Exclude) = &URenderOverride::SetupDynamics;
    INT(URenderOverride::* ClipBspSurf)(INT iNode, FTransform**& OutPts) = &URenderOverride::ClipBspSurf;
  }

  namespace URenderVTableOverrides
  {
    UBOOL(URenderOverride::* BoundVisible)(FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Result) = &URenderOverride::BoundVisible;
    FSceneNode* (URenderOverride::* CreateMasterFrame)(UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds) = &URenderOverride::CreateMasterFrame;
    void(URenderOverride::* PreRender)(FSceneNode* Frame) = &URenderOverride::PreRender;
    void(URenderOverride::* PostRender)(FSceneNode* Frame) = &URenderOverride::PostRender;
    void(URenderOverride::* DrawWorld)(FSceneNode* Frame) = &URenderOverride::DrawWorld;
  }

  /*
  * VTable for URender:
  *  [22] URender::Init
     [23] URender::PreRender
     [24] URender::PostRender
     [25] URender::CreateMasterFrame
     [26] URender::CreateChildFrame
     [27] URender::FinishMasterFrame
     [28] URender::DrawWorld
     [29] URender::DrawActor
     [30] URender::Project
     [31] URender::Deproject
     [32] URender::BoundVisible
     [33] URender::GetVisibleSurfs
     [34] URender::GlobalLighting
     [35] URender::Precache
     [36] URender::DrawCircle
     [37] URender::DrawBox
  */
  std::tuple<PLH::VFuncMap::value_type, uint64_t, PLH::VFuncMap> URenderMappedVTableFuncs[] = {
    {{(uint16_t)23, *(uint64_t*)&URenderVTableOverrides::PreRender}, (uint64_t)&URenderVTableFuncs::PreRender, {}},
    {{(uint16_t)24, *(uint64_t*)&URenderVTableOverrides::PostRender}, (uint64_t)&URenderVTableFuncs::PostRender, {}},
    {{(uint16_t)25, *(uint64_t*)&URenderVTableOverrides::CreateMasterFrame}, (uint64_t)&URenderVTableFuncs::CreateMasterFrame, {}},
    {{(uint16_t)28, *(uint64_t*)&URenderVTableOverrides::DrawWorld}, (uint64_t)&URenderVTableFuncs::DrawWorld, {}},
    {{(uint16_t)32, *(uint64_t*)&URenderVTableOverrides::BoundVisible}, (uint64_t)&URenderVTableFuncs::BoundVisible, {}},
  };
}

void InstallURenderHacks()
{
  using namespace Hacks;

  if (!URenderHacksInstalled)
  {
    URenderHacksInstalled = true;
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::OccludeBsp, *(uint64_t*)&URenderOverrides::OccludeBsp, &URenderFuncs::OccludeBsp.func64));
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::DrawFrame, *(uint64_t*)&URenderOverrides::DrawFrame, &URenderFuncs::DrawFrame.func64));
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::OccludeFrame, *(uint64_t*)&URenderOverrides::OccludeFrame, &URenderFuncs::OccludeFrame.func64));
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::DrawMesh, *(uint64_t*)&URenderOverrides::DrawMesh, &URenderFuncs::DrawMesh.func64));
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::SetupDynamics, *(uint64_t*)&URenderOverrides::SetupDynamics, &URenderFuncs::SetupDynamics.func64));
    URenderDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&URenderFuncs::ClipBspSurf, *(uint64_t*)&URenderOverrides::ClipBspSurf, &URenderFuncs::ClipBspSurf.func64));
    for (auto& detour : URenderDetours)
    {
      detour->hook();
    }



    for (auto& func : URenderMappedVTableFuncs)
    {
      auto funcDescriptor = std::get<0>(func);
      uint32_t* origFuncPtr = reinterpret_cast<uint32_t*>(std::get<1>(func));
      PLH::VFuncMap* origFuncMap = &std::get<2>(func);
      origFuncMap->clear();
      PLH::VFuncMap v = { funcDescriptor };
      auto ptr = std::make_shared<PLH::VFuncSwapHook>(reinterpret_cast<uint64_t>(GRender), v, origFuncMap);
      ptr->hook();
      *origFuncPtr = (*origFuncMap)[funcDescriptor.first];
      URenderDetours.push_back(std::move(ptr));
    }

    ///
    //Hijacking of a specific call to URender::SetupRaster, to force an early-out during occlusion.
    //SetupRaster is not virtual. 
    {
      static bool installedOnce = false;
      if (!installedOnce)
      {
        installedOnce = true;

        HMODULE renderModule = GetModuleHandleA("render.dll");
        DWORD oldProtect;

        uint8_t* callinstruction = reinterpret_cast<uint8_t*>(uint32_t(renderModule) + 0x18140);
        VirtualProtect(callinstruction, sizeof(void*) * 8, PAGE_READWRITE, &oldProtect);

        int32_t& callOffset = *(reinterpret_cast<int32_t*>(callinstruction + 1));
        uint32_t originalCallAddress = callOffset + reinterpret_cast<uint32_t>(callinstruction + 5);
        URenderFuncs::SetupRaster = reinterpret_cast<decltype(URenderFuncs::SetupRaster)>(originalCallAddress);
        uint32_t newCalladdress = reinterpret_cast<uint32_t>(&URenderOverride::SetupRaster);
        int32_t currentAddress = reinterpret_cast<int32_t>(callinstruction + 5);
        auto newOffset = static_cast<int32_t>(newCalladdress) - currentAddress;
        callOffset = newOffset;

        VirtualProtect(callinstruction, sizeof(void*) * 8, oldProtect, &oldProtect);
      }
    }
  }
}


void UninstallURenderHacks()
{
  using namespace Hacks;

  if (URenderHacksInstalled)
  {
    URenderHacksInstalled = false;
    for (auto& d : URenderDetours)
    {
      d->unHook();
    }
    URenderFuncs::OccludeBsp.Restore();
    URenderFuncs::DrawFrame.Restore();
    URenderFuncs::OccludeFrame.Restore();
    URenderFuncs::DrawMesh.Restore();
    URenderFuncs::ClipBspSurf.Restore();
    URenderFuncs::SetupDynamics.Restore();
    URenderDetours.clear();
  }
}