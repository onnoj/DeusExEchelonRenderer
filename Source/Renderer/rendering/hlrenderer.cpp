#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hlrenderer.h"
#include "renderobjectmanager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <intrin.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <stdio.h>
#include <io.h>
#include <FCNTL.H>
#include <vector>
#include <type_traits>

#include "hacks/misc.h"
#include "utils/configmanager.h"
#include "utils/debugmenu.h"
#include "utils/utils.h"
#include "MurmurHash3.h"

//////////////////////////////////////////////////////////////////////////
void HighlevelRenderer::Initialize(LowlevelRenderer* pLLRenderer)
{
  m_LLRenderer = pLLRenderer;
  m_TextureManager.Initialize(m_LLRenderer);
  m_LightManager.Initialize(m_LLRenderer);
  GLog->Log(L"[EchelonRenderer]\t HLRenderer Initialized");
}

void HighlevelRenderer::Shutdown()
{
  m_renderingScope.release();
  for(auto& n : m_DrawnNodes) n.clear();
  m_staticGeometryMeshes.clear();
  m_dynamicGeometryMeshes.clear();
  m_DebugMesh = {};
  m_LightManager.Shutdown();
  m_TextureManager.Shutdown();
  m_LLRenderer = nullptr;
  GLog->Log(L"[EchelonRenderer]\t HLRenderer shutdown");
}

//OnRenderingBegin is called before Deus Ex starts rendering the scene graph.
void HighlevelRenderer::OnRenderingBegin(const FSceneNode* Frame)
{
  m_LLRenderer->EmitDebugText(L"[EchelonRenderer] OnRenderingBegin");
  m_renderingScope = std::make_unique<FrameContextManager::ScopedContext>();

  auto& ctx = *g_ContextManager.GetContext();
  ctx.frameSceneNode = Frame;

  static uint32_t lastLevel = 0;
  uint32_t currentLevel = reinterpret_cast<uint32_t>(Frame->Level);
  bool levelChanged = (lastLevel != currentLevel);
  if (levelChanged)
  {
    lastLevel = currentLevel;
    OnLevelChange();
  }

  ctx.overrides.bypassSpanBufferRasterization = levelChanged;
  ctx.overrides.levelChanged = levelChanged;
  ctx.overrides.enableViewportXYOffsetWorkaround = g_options.enableViewportXYOffsetWorkaround;
  g_DebugMenu.DebugVar("Rendering", "Bypass SpanBuffer Rasterization", DebugMenuUniqueID(), ctx.overrides.bypassSpanBufferRasterization);

  //m_LLRenderer->beginScene();
  SetViewState(Frame, ViewType::game);
  SetProjectionState(Frame, ProjectionType::perspective);

  //Material debugging...
  m_MaterialDebugger.Update(Frame);

  //HACK, render fake UI to force early RTX injection:
  if (false)
  {
    m_LLRenderer->BeginScene();
    m_LLRenderer->PushDeviceState();
    {
      m_LLRenderer->EmitDebugText(L"[EchelonRenderer] Begin 2D UI");
      m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 0);
      m_LLRenderer->SetRenderState(D3DRS_LIGHTING, false);
      m_LLRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      m_LLRenderer->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
      m_LLRenderer->SetRenderState(D3DRS_DITHERENABLE, TRUE);
      SetViewState(Frame, ViewType::identity);
      SetProjectionState(Frame, ProjectionType::uiorthogonal);

      //Render any frame clipping (since it seems rtxremix doesn't do that for us).
      if (true)
      {
        UIntRect clipLeft, clipTop, clipRight, clipBottom;
        m_LLRenderer->GetClipRects(clipLeft, clipTop, clipRight, clipBottom);

        Draw2DScreenQuad(Frame, clipLeft.Left, clipLeft.Top, clipLeft.Width, clipLeft.Height, 0xFF000000ul);
        Draw2DScreenQuad(Frame, clipTop.Left, clipTop.Top, clipTop.Width, clipTop.Height, 0xFF000000ul);
        Draw2DScreenQuad(Frame, clipRight.Left, clipRight.Top, clipRight.Width, clipRight.Height, 0xFF000000ul);
        Draw2DScreenQuad(Frame, clipBottom.Left, clipBottom.Top, clipBottom.Width, clipBottom.Height, 0xFF000000ul);
      }
      m_LLRenderer->PopDeviceState();
      m_LLRenderer->EndScene();
    }
  }
}

void HighlevelRenderer::OnRenderingEnd(const FSceneNode* Frame)
{
  check(Frame->Parent == nullptr);

  //Render real-time lights
  m_LightManager.Render(Frame);

  auto& ctx = *g_ContextManager.GetContext();

  
  bool clearEachFrame = false;
  g_DebugMenu.DebugVar("Rendering", "Wipe Render Cache", DebugMenuUniqueID(), clearEachFrame);
  if (clearEachFrame)
  {
    m_staticGeometryMeshes.clear();
    for(auto& n : m_DrawnNodes) n.clear();
  }
  m_dynamicGeometryMeshes.clear();

#if 0
  //axis widget
  assert(Frame->Parent == nullptr);
  FColor color[3] = { FColor(255,0,0), FColor(0,255,0), FColor(0,0,255) };
  FVector axis[3] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };
  auto widgetPos = Frame->Coords.Origin - ((Frame->Coords.XAxis ^ Frame->Coords.YAxis) * 50.0f);
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
  widgetPos.X *= -1.0f;
#endif
  for (int i = 0; i < 3; i++)
  {
    Draw3DLine(Frame, widgetPos, widgetPos + (10.0f * axis[i]), color[i]);
  }
#endif

  //Render debug crap:
  if (m_DebugMesh.primitiveCount > 0)
  {
    m_LLRenderer->PushDeviceState();
    m_LLRenderer->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
    m_LLRenderer->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    m_LLRenderer->SetRenderState(D3DRS_ZENABLE, 0);
    m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 1);
    m_LLRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_LLRenderer->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_DIFFUSE);
    m_LLRenderer->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    SetWorldTransformStateToIdentity();
#if 0
    m_LLRenderer->RenderTriangleListBuffer(
      D3DFVF_XYZ | D3DFVF_DIFFUSE /*| D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
      m_DebugMesh.buffer.data(),
      m_DebugMesh.primitiveCount,
      m_DebugMesh.buffer.size(),
      sizeof(VertexPos3Color0),
      0,
      0xdeafbeef
    );
#endif
    m_DebugMesh.buffer.clear();
    m_DebugMesh.primitiveCount = 0;
    m_LLRenderer->PopDeviceState();
  };

  m_LLRenderer->BeginScene();
    m_LLRenderer->PushDeviceState();
    {
      //Override viewport depth to signal rtxremix that we're rendering a viewmodel (weapon).
      //Otherwise, it will show up in reflections and shadow.
      m_LLRenderer->EmitDebugText(L"[EchelonRenderer] Begin 3D UI");
      m_LLRenderer->SetViewportDepth(RenderRanges::UI /*viewmodel?*/);
      m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 0);
      m_LLRenderer->SetRenderState(D3DRS_LIGHTING, false);
      m_LLRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      m_LLRenderer->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
      m_LLRenderer->SetRenderState(D3DRS_DITHERENABLE, TRUE);
      m_LLRenderer->ClearDepth();
      SetViewState(Frame, ViewType::game);
      SetProjectionState(Frame, ProjectionType::perspective);
      ExecuteCommandQueue(&ctx, RenderCommandQueue::uiMesh);
    }
    m_LLRenderer->PopDeviceState();
  m_LLRenderer->PushDeviceState();
  {
    m_LLRenderer->EmitDebugText(L"[EchelonRenderer] Begin 2D UI");
    m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    m_LLRenderer->SetRenderState(D3DRS_LIGHTING, false);
    m_LLRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_LLRenderer->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    m_LLRenderer->SetRenderState(D3DRS_DITHERENABLE, TRUE);
    m_LLRenderer->ClearDepth();
    SetViewState(Frame, ViewType::identity);
    SetProjectionState(Frame, ProjectionType::uiorthogonal);


    ExecuteCommandQueue(&ctx, RenderCommandQueue::ui);

  }
  m_LLRenderer->PopDeviceState();

  ClearCommandQueue(RenderCommandQueue::ui);
  ClearCommandQueue(RenderCommandQueue::uiMesh);
  ClearCommandQueue(RenderCommandQueue::mapGeometry);
  ClearCommandQueue(RenderCommandQueue::dynamicMesh);
  ClearCommandQueue(RenderCommandQueue::pfx);
  ClearCommandQueue(RenderCommandQueue::mapGeometryTransparent);

  m_RenderObjectManager.ResetRenderObjects(RenderObjectLifetime::Instant);
  m_RenderObjectManager.ResetRenderObjects(RenderObjectLifetime::Frame);

  m_LLRenderer->EndScene();

  //Render Remix warning screen
  if (!IsDebuggerPresent())
  {
    //DrawFullscreenQuad(Frame, m_TextureManager.GetFakeTexture());
  }


  m_LLRenderer->EmitDebugText(L"[EchelonRenderer] OnRenderingEnd");
  m_renderingScope.reset();
}

void HighlevelRenderer::DrawPlayerBody(const FSceneNode* Frame)
{
  auto& ctx = *g_ContextManager.GetContext();
  bool renderPlayerBody = (!ctx.frameIsSkybox && !ctx.renderingUI && Frame->Parent == nullptr);
  if (renderPlayerBody)
  {
    //Draw player body.
    auto player = Frame->Viewport->Actor;
    if (g_ConfigManager.GetRenderPlayerBody() && !player->bBehindView)
    {
      static UBOOL& HasSpecialCoords = []() -> UBOOL& {
        uint32_t baseAddress = (uint32_t)GetModuleHandle(L"Render.dll");
        return *reinterpret_cast<UBOOL*>(baseAddress + 0x4eb10);
        }();
      static FCoords& SpecialCoords = []() -> FCoords& {
        uint32_t baseAddress = (uint32_t)GetModuleHandle(L"Render.dll");
        return *reinterpret_cast<FCoords*>(baseAddress + 0x4ea08);
        }();


      BOOL originalHasSpecialCoords = HasSpecialCoords;
      BITFIELD originalBehindView = player->bBehindView;
      {
        HasSpecialCoords = 0;
        player->bBehindView = 1;
        GRender->DrawMesh(
          const_cast<FSceneNode*>(Frame),
          player,
          player,
          nullptr,
          nullptr,
          Frame->Coords,
          nullptr,
          nullptr,
          PF_TwoSided|PF_NoMerge
        );
      }
      player->bBehindView = originalBehindView;
      HasSpecialCoords = originalHasSpecialCoords;
    }
  }
}

//OnSceneBegin is called when Deus Ex starts rendering a specific scene (as determined by a camera)
//In modern rasterization engines, games with multiple cameras would render each camera to texture
//and then in then finally those textures would be used in the final frame.
//In UE1, multiple cameras is done through portal rendering. When the renderer encounters a
//portal surface, it switches camera and continues to draw within the shape of the portal surface.
//Sadly, camera switching is not quite supported with RTX Remix.
void HighlevelRenderer::OnSceneBegin(const FSceneNode* Frame)
{
  auto& ctx = *g_ContextManager.GetContext();
  m_LLRenderer->EmitDebugText(L"[EchelonRenderer] OnSceneBegin");

  if (Frame->Parent == nullptr)
  {
    g_Stats.Writer().DrawMainFrame();
  }

  if (ctx.frameIsRasterized)
  {
    ctx.overrides.enableViewportXYOffsetWorkaround = false;
  }

  if (ctx.frameIsSkybox && Frame->Parent != nullptr)
  {
    m_LLRenderer->EmitDebugText(L"[EchelonRenderer] Skybox begin");

    //rtx requires that the full pass is rendered with the same view matrix.
    //ie, we cannot change cameras.
    //since the matrix multiplication is: world * view * proj.
    //we can turn this into (world * subframeView * invRootView) * rootview * projection
    ctx.skyframeSceneNode = std::make_shared<FSceneNode>(*Frame);

    //So, there's a weird problem where the skybox camera rotates around up-axis, but not side-axis.
    //Bit of a hack but it saves some headache...
    AZoneInfo* SkyZone = (AZoneInfo*)Frame->Level->GetZoneActor(Frame->ZoneNumber)->SkyZone;
    FCoords SkyCoords = Frame->Parent->Coords;
    SkyCoords *= Frame->Parent->Coords.Origin;
    SkyCoords /= SkyZone->Rotation;
    SkyCoords /= SkyZone->Location;
    const_cast<FSceneNode*>(Frame)->Coords = SkyCoords;
    g_Stats.Writer().DrawSkyBox();
  }
  //  
  m_LLRenderer->PushDeviceState();
  m_LLRenderer->BeginScene();

  if (ctx.frameIsSkybox)
  {
    m_LLRenderer->SetViewportDepth(RenderRanges::Skybox);
    m_LLRenderer->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_LLRenderer->SetRenderState(D3DRS_LIGHTING, FALSE);
  }
  else
  {
    m_LLRenderer->SetViewportDepth(RenderRanges::Game);
  }
}

void HighlevelRenderer::OnSceneEnd(const FSceneNode* Frame)
{
  auto& ctx = *g_ContextManager.GetContext();
  Utils::ScopedCall scopedCall
  {
    [&]() {},
    [&]()
    {
      m_LLRenderer->EndScene();
      m_LLRenderer->PopDeviceState();
      m_LLRenderer->EmitDebugText(L"[EchelonRenderer] OnSceneEnd");
    }
  };

  bool renderMainPass = true;
  bool renderSprites = (!ctx.frameIsSkybox && !ctx.renderingUI && Frame->Parent == nullptr);
  if ((ctx.frameIsSkybox && !g_ConfigManager.GetRenderSkybox()))
  {
    renderMainPass = false;
  }

  if (renderMainPass)
  {
    m_LLRenderer->PushDeviceState();
    SetProjectionState(Frame, HighlevelRenderer::ProjectionType::perspective);
    SetViewState(Frame, ViewType::game);
    ExecuteCommandQueue(&ctx, RenderCommandQueue::mapGeometry);
    m_LLRenderer->PopDeviceState();

    m_LLRenderer->PushDeviceState();
    SetProjectionState(Frame, HighlevelRenderer::ProjectionType::perspective);
    SetViewState(Frame, ViewType::game);
    ExecuteCommandQueue(&ctx, RenderCommandQueue::mapGeometryTransparent);
    m_LLRenderer->PopDeviceState();

    m_LLRenderer->PushDeviceState();
    SetProjectionState(Frame, HighlevelRenderer::ProjectionType::perspective);
    SetViewState(Frame, ViewType::game);
    ExecuteCommandQueue(&ctx,RenderCommandQueue::dynamicMesh);
    m_LLRenderer->PopDeviceState();
  }
  //
  if (renderSprites)
  {
    m_LLRenderer->PushDeviceState();
    SetViewState(Frame, ViewType::game);
    SetProjectionState(Frame, ProjectionType::perspective);
    ExecuteCommandQueue(&ctx,RenderCommandQueue::pfx);
    m_LLRenderer->PopDeviceState();
  }
}

#if 0
void HighlevelRenderer::OnSceneEnd(const FSceneNode* Frame)
{
  auto& ctx = *g_ContextManager.GetContext();
  Utils::ScopedCall scopedCall
  {
    [&]() {},
    [&]()
    {
      m_LLRenderer->EndScene();
      m_LLRenderer->PopDeviceState();
      m_LLRenderer->EmitDebugText(L"[EchelonRenderer] OnSceneEnd");
    }
  };

  if (!ctx.frameIsSkybox || (ctx.frameIsSkybox && g_ConfigManager.GetRenderSkybox()))
  {
    UModel* Model = Frame->Level->Model;
    auto& GSurfs = Model->Surfs;
    auto& GNodes = Model->Nodes;
    auto& GVerts = Model->Verts;
    auto& GVertPoints = Model->Points;

    ExecuteCommandQueue(RenderCommandQueue::staticGeo);
    ExecuteCommandQueue(RenderCommandQueue::mapGeometry);

    {  //Render all static geometry
      enum class pass { solid, translucent };
      constexpr pass passes[] = { pass::solid, pass::translucent };
      GeometryMeshesMap* buckets[] = { &m_staticGeometryMeshes, &m_dynamicGeometryMeshes};

      for (auto bucketPtr : buckets)
      {
        for (auto pass : passes)
        {
          GeometryMeshesMap& bucket = *bucketPtr;
          for (auto& cachedMeshInfoIt : bucket)
          {
            //Utils::ScopedCall scopedRenderStatePushPop{ [&](){ m_LLRenderer->PushDeviceState();}, [&]() {m_LLRenderer->PopDeviceState(); } };

            auto& info = cachedMeshInfoIt.second;
            auto& vtxBuffer = *info.buffer.get();
            if (vtxBuffer.empty())
            {
              continue;
            }

            if (!info.zoneIndices.test(Frame->ZoneNumber))
            {
              continue;
            }

            auto flags = info.flags;
            const bool isTranslucent = (flags & PF_Translucent) != 0;
            const bool isUnlitEmissive = ((flags & PF_Unlit) != 0);
            auto wm = info.worldMatrix;

            if (pass == pass::solid)
            {
              if (isTranslucent)
              {
                continue;
              }

              if (isUnlitEmissive)
              { //render without unlit flag
                flags &= ~PF_Unlit;
              }
            }
            else if (pass == pass::translucent)
            {
              if (!(isTranslucent || isUnlitEmissive))
              {
                continue;
              }

              if (isUnlitEmissive)
              {
                //Additional 2x renders with a tiny offset, otherwise the effect might not get picked up per-ray.
                if (!ctx.frameIsSkybox)
                {
                  D3DXMATRIX s;
                  D3DXMatrixScaling(&s, 1.0001f, 1.0001f, 1.0001f);
                  D3DXMatrixMultiply(&wm, &info.worldMatrix, &s);
                  SetWorldTransformState(wm);
                  m_TextureManager.BindTexture(flags, info.albedoTextureHandle, info.lightmapTextureHandle);
                  m_LLRenderer->RenderTriangleList(info.buffer->data(), info.primitiveCount, info.buffer->size(), info.hash, info.debug);

                  D3DXMatrixScaling(&s, 0.9999f, 0.9999f, 0.9999f);
                  D3DXMatrixMultiply(&wm, &info.worldMatrix, &s);
                  SetWorldTransformState(wm);
                  m_TextureManager.BindTexture(flags, info.albedoTextureHandle, info.lightmapTextureHandle);
                  m_LLRenderer->RenderTriangleList(info.buffer->data(), info.primitiveCount, info.buffer->size(), info.hash, info.debug);
                  continue;
                }
              }
            }

            SetWorldTransformState(wm);
            if (m_TextureManager.BindTexture(flags, info.albedoTextureHandle, info.lightmapTextureHandle))
            {
              m_LLRenderer->RenderTriangleList(info.buffer->data(), info.primitiveCount, info.buffer->size(), info.hash, info.debug);
            }
          }
        }
      }
    }
  }
}
#endif

void HighlevelRenderer::Draw2DScreenQuad(const FSceneNode* Frame, float pX, float pY, float pWidth, float pHeight, uint32_t pARGB/* = 0xFF000000ul*/)
{
  auto [ro,roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos3Color0>(0, RenderObjectLifetime::Instant);
  auto quad = ro->AcquireBuffer<VertexPos3Color0>();

  quad->Assign({
    { { pX+0.0f,    pY+0.0f,    1.0f }, {pARGB} },
    { { pX+pWidth,	pY+pHeight, 1.0f }, {pARGB} },
    { { pX+0.0f,    pY+pHeight, 1.0f }, {pARGB} },
    { { pX+0.0f,    pY+0.0f,    1.0f }, {pARGB} },
    { { pX+pWidth,	pY+0.0f,    1.0f }, {pARGB} },
    { { pX+pWidth,	pY+pHeight, 1.0f }, {pARGB} }
  });

  AddRenderCommand(RenderCommandQueue::ui, [this, ro](const FrameContextManager::Context* pContext) {
    SetWorldTransformStateToIdentity();
    m_LLRenderer->ConfigureBlendState(0);
    m_LLRenderer->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_DIFFUSE);
    m_LLRenderer->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    m_LLRenderer->Render(ro.get());
  });
}

void HighlevelRenderer::Draw3DCube(const FSceneNode* Frame, const FVector& Position, DWORD pPrimitiveFlags, const DeusExD3D9TextureHandle& pTexture,float Size/*=1.0f*/)
{
  if (!g_options.hasDebugDraw)
  {
    return;
  }
  m_LLRenderer->PushDeviceState();
  VertexPos3Tex0 vertices[] = {
    // Front face
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},  // Bottom left
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},   // Bottom right
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    // Top right
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},   // Top left

    // Back face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, // Bottom left
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},  // Bottom right
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},   // Top right
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}}   // Top left
  };

  // Define the indices for the triangles
  const uint32_t indices[] = {
    // Front face
    0, 1, 2, // First triangle
    2, 3, 0, // Second triangle

    // Right face
    1, 5, 6, // First triangle
    6, 2, 1, // Second triangle

    // Back face
    5, 4, 7, // First triangle
    7, 6, 5, // Second triangle

    // Left face
    4, 0, 3, // First triangle
    3, 7, 4, // Second triangle

    // Bottom face
    4, 5, 1, // First triangle
    1, 0, 4, // Second triangle

    // Top face
    3, 2, 6, // First triangle
    6, 7, 3  // Second triangle
  };

  D3DXMATRIX wm;
  D3DXMatrixTransformation(&wm, nullptr, nullptr, &D3DXVECTOR3(Size, Size, Size), nullptr, nullptr, &D3DXVECTOR3(
    Position.X, Position.Y, Position.Z
  ));
  SetWorldTransformState(wm);

  auto [ro,roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos3Tex0>(0, RenderObjectLifetime::Instant);
  auto buffer = ro->AcquireBuffer<VertexPos3Tex0>();

  const auto faces = std::size(indices) / 3;
  for (int i = 0; i < faces; ++i) {
    buffer->PushFace({vertices[indices[(i * 3) + 0]], vertices[indices[(i * 3) + 1]], vertices[indices[(i * 3) + 2]]});
  }
  if (m_TextureManager.BindTexture(pPrimitiveFlags, pTexture))
  {
    m_LLRenderer->Render(ro.get());
  }
  m_LLRenderer->PopDeviceState();
}

void HighlevelRenderer::Draw3DLine(const FSceneNode* Frame, const FVector& PositionFrom, const FVector& PositionTo, FColor Color, float Size/* = 1.0f*/)
{
  if (!g_options.hasDebugDraw)
  {
    return;
  }

  FVector lineDir = (PositionTo - PositionFrom); lineDir.Normalize();
  FVector cameraToLineDir = (Frame->Coords.Origin - PositionFrom);
  if (cameraToLineDir.Normalize() == 0)
  {
    cameraToLineDir = -Frame->Coords.ZAxis;
  }
  FVector lineSides = lineDir ^ cameraToLineDir; lineSides.Normalize();

  const FVector triangle1[3] = { (PositionFrom - lineSides), (PositionTo - lineSides), (PositionFrom + lineSides) };
  const FVector triangle2[3] = { (PositionTo - lineSides), (PositionFrom + lineSides), (PositionTo + lineSides) };

  m_DebugMesh.buffer.push_back({ {triangle1[0].X, triangle1[0].Y, triangle1[0].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.buffer.push_back({ {triangle1[1].X, triangle1[1].Y, triangle1[1].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.buffer.push_back({ {triangle1[2].X, triangle1[2].Y, triangle1[2].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.primitiveCount++;
  m_DebugMesh.buffer.push_back({ {triangle2[0].X, triangle2[0].Y, triangle2[0].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.buffer.push_back({ {triangle2[1].X, triangle2[1].Y, triangle2[1].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.buffer.push_back({ {triangle2[2].X, triangle2[2].Y, triangle2[2].Z}, 0xFF000000 | Color.TrueColor() });
  m_DebugMesh.primitiveCount++;
}

void HighlevelRenderer::DrawFullscreenQuad(const FSceneNode* Frame, const DeusExD3D9TextureHandle& pTexture)
{
  m_LLRenderer->PushDeviceState();
  {
    D3DXMATRIX wm; D3DXMatrixIdentity(&wm);
    D3DXMATRIX wmi; D3DXMatrixIdentity(&wmi);
    D3DXMATRIX identity; D3DXMatrixIdentity(&identity);

    FVector newOrigin = Frame->Coords.Origin - ((Frame->Coords.XAxis ^ Frame->Coords.YAxis) * 1.0f);

    FCoords coords;
    coords.Origin = newOrigin;
    coords.XAxis = Frame->Coords.XAxis;
    coords.YAxis = Frame->Coords.YAxis * -1.0f;
    coords.ZAxis = Frame->Coords.ZAxis;
    wm = UECoordsToMatrix(coords, &wmi);

    D3DXMatrixTranslation(&wm, newOrigin.X, newOrigin.Y, newOrigin.Z);

    auto [ro,roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos3Tex0>(0, RenderObjectLifetime::Instant);
    auto buffer = ro->AcquireBuffer<VertexPos3Tex0>();

    buffer->PushFace({
        { { -1.0f, -1.0f, 0.0f },  {0.0f, 1.0f} },
        { {  1.0f,	 1.0f, 0.0f },  {1.0f, 0.0f} },
        { { -1.0f,  1.0f, 0.0f },  {0.0f, 0.0f} },
      });

    buffer->PushFace({
        { { -1.0f, -1.0f, 0.0f },  {0.0f, 1.0f} },
        { {  1.0f,	-1.0f, 0.0f },  {1.0f, 1.0f} },
        { {  1.0f,	 1.0f, 0.0f },  {1.0f, 0.0f} }
      });

    if (m_TextureManager.BindTexture(PF_Unlit, pTexture))
    {
      m_LLRenderer->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
      m_LLRenderer->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
      m_LLRenderer->SetRenderState(D3DRS_ZENABLE, 0);
      m_LLRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 1);
      m_LLRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

      SetWorldTransformState(wmi);
      m_LLRenderer->Render(ro.get());
    }
  }
  m_LLRenderer->PopDeviceState();
}

void HighlevelRenderer::OnLevelChange()
{
  m_staticGeometryMeshes.clear();
  m_dynamicGeometryMeshes.clear();
  m_RenderObjectManager.ResetRenderObjects(RenderObjectLifetime::Frame);
  m_RenderObjectManager.ResetRenderObjects(RenderObjectLifetime::Level);
  for(auto& n : m_DrawnNodes) n.clear();
  m_LightManager.OnLevelChange();
}

void HighlevelRenderer::GetViewMatrix(const FCoords& FrameCoords, D3DXMATRIX& viewMatrix)
{
  D3DXMatrixIdentity(&viewMatrix);
  FVector translation = FVector(0.0f, 0.0f, 0.0f);
  translation = translation.TransformPointBy(FrameCoords);

  D3DXMATRIX initialMatrix;
  D3DXMatrixIdentity(&initialMatrix);
  {
    initialMatrix(0, 0) = 1.0f;
    initialMatrix(0, 1) = 0.0f;
    initialMatrix(0, 2) = 0.0f;

    initialMatrix(1, 0) = 0.0f;
    initialMatrix(1, 1) = -1.0f;
    initialMatrix(1, 2) = 0.0f;

    initialMatrix(2, 0) = 0.0f;
    initialMatrix(2, 1) = 0.0f;
    initialMatrix(2, 2) = 1.0f;
  }

  D3DXMATRIX cameraMatrix;
  D3DXMatrixIdentity(&cameraMatrix);
  {
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
    FVector axis[3]{ FVector(-1.0f,0.0f,0.0f),FVector(0.0f, 1.0f, 0.0f),FVector(0.0f, 0.0f, 1.0f) };
#else
    FVector axis[3]{ FVector(1.0f,0.0f,0.0f),FVector(0.0f, 1.0f, 0.0f),FVector(0.0f, 0.0f, 1.0f) };
#endif
    axis[0] = axis[0].TransformVectorBy(FrameCoords);
    axis[1] = axis[1].TransformVectorBy(FrameCoords);
    axis[2] = axis[2].TransformVectorBy(FrameCoords);
    check(UEFloatEquals(axis[0].SizeSquared(), 1.0f));
    check(UEFloatEquals(axis[1].SizeSquared(), 1.0f));
    check(UEFloatEquals(axis[2].SizeSquared(), 1.0f));
    //axis[0].Normalize();
    //axis[1].Normalize();
    //axis[2].Normalize();

    cameraMatrix(0, 0) = axis[0].X;
    cameraMatrix(0, 1) = axis[0].Y;
    cameraMatrix(0, 2) = axis[0].Z;

    cameraMatrix(1, 0) = axis[1].X;
    cameraMatrix(1, 1) = axis[1].Y;
    cameraMatrix(1, 2) = axis[1].Z;

    cameraMatrix(2, 0) = axis[2].X;
    cameraMatrix(2, 1) = axis[2].Y;
    cameraMatrix(2, 2) = axis[2].Z;

    FVector translation = FVector(0.0f, 0.0f, 0.0f);
    translation = translation.TransformPointBy(FrameCoords);
    cameraMatrix(3, 0) = translation.X;
    cameraMatrix(3, 1) = translation.Y;
    cameraMatrix(3, 2) = translation.Z;
  }

  D3DXMatrixMultiply(&viewMatrix, &cameraMatrix, &initialMatrix);
}

void HighlevelRenderer::GetPerspectiveProjectionMatrix(const FSceneNode* Frame, D3DXMATRIX& projMatrix)
{
  /*
  * At the time of writing, RTX Remix does not honor the X and Y offsets of the viewport
  * As a hack, I correct the perspective instead.
  */
  auto ctx = g_ContextManager.GetContext();
  bool useFrame = (!ctx->overrides.enableViewportXYOffsetWorkaround || (Frame->XB == 0 && Frame->YB == 0));

  const RangeDefinition& renderRanges = RenderRanges::FromContext(ctx);

  const float fovangle = Frame->Viewport->Actor->FovAngle;
  const float aspect = (useFrame ? Frame->FY / Frame->FX : float(Frame->Viewport->SizeY) / float(Frame->Viewport->SizeX));
  const float fovHalfAngle = appTan(fovangle * (PI / 360.0f)) * renderRanges.NearRange;

  float viewportW = useFrame ? float(Frame->FX) : float(Frame->Viewport->SizeX);
  float viewportH = useFrame ? float(Frame->FY) : float(Frame->Viewport->SizeY);
  float offsetL = (float(Frame->XB) / viewportW) * 0.5f;
  float offsetT = (float(Frame->YB) / viewportH) * 0.5f;
  float offsetR = (1.0f - (float(Frame->XB + Frame->X) / viewportW)) * 0.5f;
  float offsetB = (1.0f - (float(Frame->YB + Frame->Y) / viewportH)) * 0.5f;

#if 1
  D3DXMatrixPerspectiveOffCenterLH(&projMatrix,
    (-1.0f + (useFrame ? 0.0f : offsetL)) * fovHalfAngle,
    (1.0f + (useFrame ? 0.0f : offsetR)) * fovHalfAngle,
    (-1.0f - (useFrame ? 0.0f : offsetB)) * aspect * fovHalfAngle,
    (1.0f - (useFrame ? 0.0f : offsetT)) * aspect * fovHalfAngle,
    renderRanges.NearRange, renderRanges.FarRange);
#else
  float zScale = 0.1f;

  D3DXMATRIX translation1, translation2;
  D3DXMatrixTranslation(&translation1, 0, 0, -renderRanges.NearRange);
  D3DXMatrixTranslation(&translation2, 0, 0, +renderRanges.NearRange);

  D3DXMATRIX scaledM;
  D3DXMatrixScaling(&scaledM, 1.0f, 1.0f, zScale);

  D3DXMATRIX persp;
  D3DXMatrixPerspectiveOffCenterLH(&persp,
    (-1.0f + (useFrame ? 0.0f : offsetL)) * fovHalfAngle / zScale,
    (1.0f + (useFrame ? 0.0f : offsetR)) * fovHalfAngle / zScale,
    (-1.0f - (useFrame ? 0.0f : offsetB)) * aspect * fovHalfAngle / zScale,
    (1.0f - (useFrame ? 0.0f : offsetT)) * aspect * fovHalfAngle / zScale,
    renderRanges.NearRange, renderRanges.FarRange
    //0.0f, 1.0f
  );

  //proj = translation1 * scale * translation2 * persp;
  D3DXMatrixIdentity(&projMatrix);
  D3DXMatrixMultiply(&projMatrix, &projMatrix, &translation1);
  D3DXMatrixMultiply(&projMatrix, &projMatrix, &scaledM);
  D3DXMatrixMultiply(&projMatrix, &projMatrix, &translation2);
  D3DXMatrixMultiply(&projMatrix, &projMatrix, &persp);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HighlevelRenderer::OnDrawGeometryBegin(const FSceneNode* Frame)
{
}

void HighlevelRenderer::OnDrawGeometry(const FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
  auto& ctx = *g_ContextManager.GetContext();

  if (Facet.Polys == nullptr)
  {
    return;
  }

  if ((Surface.PolyFlags & PF_Mirrored) != 0)
  {
    Surface.PolyFlags &= ~PF_Invisible;
  }

  if ((Surface.PolyFlags & PF_Invisible) != 0)
  {
    return;
  }

  UModel* Model = Frame->Level->Model;
  auto& GSurfs = Model->Surfs;
  auto& GNodes = Model->Nodes;
  auto& GVerts = Model->Verts;
  auto& GVertPoints = Model->Points;
  const INT	iNode = Facet.Polys[0].iNode;
  const auto& Node = GNodes(iNode);

  bool surfaceIsDynamic = [&]() 
    {
      if (ctx.frameIsSkybox)
      {
        //Consider the skybox dynamic; depth buffering is
        //turned off, so we can't cache the render calls.
        return true;
      }

      if (Frame->Level->BrushTracker != nullptr)
      {
        if (Frame->Level->BrushTracker->SurfIsDynamic(Node.iSurf))
        {
          return true;
        }
      }

      if (Surface.Texture->bRealtimeChanged || Surface.Texture->bRealtime)
      {
        return true;
      }

      if ((Surface.PolyFlags & (PF_AutoUPan | PF_AutoVPan)) != 0)
      {
        return true;
      }
    return false;
    }();

  D3DXMATRIX worldMatrix, worldMatrixInverse;
  FVector localOrigin = GVertPoints(GVerts(Node.iVertPool + 0).pVertex);
  D3DXMatrixTranslation(&worldMatrix, localOrigin.X, localOrigin.Y, localOrigin.Z);
  D3DXMatrixInverse(&worldMatrixInverse, nullptr, &worldMatrix);

  DeusExD3D9TextureHandle albedoTextureHandle;
  DeusExD3D9TextureHandle lightmapTextureHandle;
  albedoTextureHandle = m_TextureManager.ProcessTexture(Surface.PolyFlags, Surface.Texture);
  if (Surface.LightMap && ctx.frameIsRasterized)
  {
    lightmapTextureHandle = m_TextureManager.ProcessTexture(Surface.PolyFlags, Surface.LightMap);
  }
  m_TextureManager.BindTexture(Surface.PolyFlags, albedoTextureHandle, lightmapTextureHandle);

  auto [ro, roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos3Tex0Tex1>(Utils::CalculateKey(iNode), surfaceIsDynamic ? RenderObjectLifetime::Frame : RenderObjectLifetime::Level);

  if (roCreated)
  {
    auto vtxBuffer = ro->AcquireBuffer<VertexPos3Tex0Tex1>();
    for (auto Poly = Facet.Polys; Poly; Poly = Poly->Next)
    {
      INT	  		polyINode = Poly->iNode;
      FBspNode* Node = &GNodes(iNode);
      INT       numPts = Node->NumVertices;
      FBspSurf* Surf = &GSurfs(Node->iSurf);
      FCoords   FrameCoords = Frame->Coords;
      assert(polyINode == iNode);
      if (numPts < 3)
      {
        continue;
      }

      FLOAT UDot = Facet.MapCoords.XAxis | Facet.MapCoords.Origin;
      FLOAT VDot = Facet.MapCoords.YAxis | Facet.MapCoords.Origin;

      //TODO: can this be cached? Or delayed until we actually need to calculate it?
      auto calculateUV = [&Facet, UDot, VDot, iNode](const FVector& pts, const FTextureInfo* pTextureInfo, const DeusExD3D9TextureHandle& pTextureHandle) -> FVector {
        if (pTextureInfo != nullptr && pTextureHandle != nullptr)
        {
          FLOAT U = Facet.MapCoords.XAxis | pts;
          FLOAT V = Facet.MapCoords.YAxis | pts;
          FLOAT ucoord = ((U - UDot) - pTextureInfo->Pan.X) * pTextureHandle->md.multU;
          FLOAT vcoord = ((V - VDot) - pTextureInfo->Pan.Y) * pTextureHandle->md.multV;
          return FVector(ucoord, vcoord, 0);
        }
        return FVector(0, 0, 0);
        };

      D3DXVECTOR4 point1;
      D3DXVECTOR4 point2;
      D3DXVECTOR4 point3;

      FVector localPts[3] = {};
      FVector projPts[3] = {};
      localPts[0] = GVertPoints(GVerts(Node->iVertPool + 0).pVertex);
      projPts[0] = localPts[0].TransformPointBy(FrameCoords);
      for (INT i = 2; i < numPts; i++)
      {
        localPts[1] = GVertPoints(GVerts(Node->iVertPool + i - 1).pVertex);
        projPts[1] = localPts[1].TransformPointBy(FrameCoords);
        localPts[2] = GVertPoints(GVerts(Node->iVertPool + i).pVertex);
        projPts[2] = localPts[2].TransformPointBy(FrameCoords);

        for (int i = 0; i < 3; i++)
        {
          const auto uvDiffuse = calculateUV(projPts[i], Surface.Texture, albedoTextureHandle);
          const auto uvLightmap = calculateUV(projPts[i], Surface.LightMap, lightmapTextureHandle);

#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
          VertexPos3Tex0Tex1 vtx = { { -localPts[i].X, localPts[i].Y, localPts[i].Z }, /*0xFF00FF00,*/{ uvDiffuse.X, uvDiffuse.Y }, {uvLightmap.X, uvLightmap.Y} };
#else
          VertexPos3Tex0Tex1 vtx = { {  localPts[i].X, localPts[i].Y, localPts[i].Z }, /*0xFF00FF00,*/{ uvDiffuse.X, uvDiffuse.Y }, {uvLightmap.X, uvLightmap.Y} };
#endif

          D3DXVec3TransformCoord(&vtx.Pos, &vtx.Pos, &worldMatrixInverse);
          vtxBuffer->PushVertex(std::move(vtx));
        }
      }
    }
  }
  
  {
    auto flags = Surface.PolyFlags; //??? check original
    auto debugId = iNode;
    const bool isTranslucent = (flags & PF_Translucent) != 0;
    const bool isUnlitEmissive = ((flags & PF_Unlit) != 0);

    if (!isTranslucent)
    {
      AddRenderCommand(RenderCommandQueue::mapGeometry, [=](const FrameContextManager::Context* pContext) {
        auto renderFlags = flags;
        renderFlags &= ~PF_Unlit;
        SetWorldTransformState(worldMatrix);
        if (m_TextureManager.BindTexture(renderFlags, albedoTextureHandle, lightmapTextureHandle))
        {
          m_LLRenderer->EmitDebugTextF(L"Render surface 0x%X with texture 0x%X (0x%X)", debugId, albedoTextureHandle.get(), albedoTextureHandle->textureD3D9);
          m_LLRenderer->Render(ro.get());
        }
      });
    }

    if (isTranslucent || isUnlitEmissive)
    {
      AddRenderCommand(RenderCommandQueue::mapGeometryTransparent, [=](const FrameContextManager::Context* pContext) {
        auto renderFlags = flags;
        D3DXMATRIX wm = worldMatrix;
        if (!pContext->frameIsSkybox)
        {
          D3DXMATRIX s;
          D3DXMatrixScaling(&s, 1.0001f, 1.0001f, 1.0001f);
          D3DXMatrixMultiply(&wm, &worldMatrix, &s);
          SetWorldTransformState(wm);
          if (m_TextureManager.BindTexture(renderFlags, albedoTextureHandle, lightmapTextureHandle))
          {
            m_LLRenderer->EmitDebugTextF(L"Render surface 0x%X with texture 0x%X (0x%X)", debugId, albedoTextureHandle.get(), albedoTextureHandle->textureD3D9);
            m_LLRenderer->Render(ro.get());
          }

          D3DXMatrixScaling(&s, 0.9999f, 0.9999f, 0.9999f);
          D3DXMatrixMultiply(&wm, &worldMatrix, &s);
          SetWorldTransformState(wm);
          if (m_TextureManager.BindTexture(renderFlags, albedoTextureHandle, lightmapTextureHandle))
          {
            m_LLRenderer->EmitDebugTextF(L"Render surface 0x%X with texture 0x%X (0x%X)", debugId, albedoTextureHandle.get(), albedoTextureHandle->textureD3D9);
            m_LLRenderer->Render(ro.get());
          }
        }
      });
    }

  }
}

void HighlevelRenderer::OnDrawGeometryEnd(const FSceneNode* Frame)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HighlevelRenderer::OnDrawMeshBegin(const FSceneNode* Frame, AActor* Owner)
{
  auto renderContext = g_ContextManager.GetContext();
  if (!renderContext->drawcallInfo)
  {
    //for now, skip any polygons not draw by DrawMesh
    return;
  }

  renderContext->drawcallInfo->Owner = Owner;
}

void HighlevelRenderer::OnDrawMeshPolygon(const FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span)
{
  if (g_options.renderingDisabled)
  {
    return;
  }

  if (NumPts < 3 || !g_options.hasObjectMeshes) //Invalid triangle
    return;

  auto renderContext = g_ContextManager.GetContext();
  if (!renderContext->drawcallInfo)
  {
    //for now, skip any polygons not draw by DrawMesh
    return;
  }
  auto owner = renderContext->drawcallInfo->Owner;
  check(owner != nullptr && owner->DrawScale >= -0.0f);


  UTexture* prevTexture = renderContext->drawcallInfo->LastTextureInfo;
  if (prevTexture != Info.Texture)
  {
    //renderContext->drawcallInfo->LastTextureMetadata.emplace(m_TextureManager.OLDGetTextureMetadata(&Info));
    renderContext->drawcallInfo->LastTextureMetadata.emplace(m_TextureManager.ProcessTexture(PolyFlags, &Info)->md);
    renderContext->drawcallInfo->LastTextureInfo = Info.Texture;
    //if (prevTexture != nullptr)
    //{
    //	//texture changed! debug me!
    //	int x = 1;
    //}
  }

  //TODO:
  //We'll have to figure out if we're going to re-use this mesh as-is, or if it needs to be re-built.
  //Dynamic meshes will need to be resubmited each frame.
  //Static meshes can be reused.

  //UE's begin-draw-end calls emit a single mesh, but we'd like to split them in seperate buffers with separate render calls.
  //Some parts might be emissive, for example. We have to perform some bookkeeping here to make sure we're able to find
  //all buffers for a given mesh.
  const RenderObjectKey key = Utils::CalculateKey(renderContext->drawcallInfo->Owner, Info.CacheID, PolyFlags);
  auto [ro, roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos3Norm3Tex0>(key, RenderObjectLifetime::Frame);
  if (roCreated)
  {
    ro->SetTexture(RenderObjectTextureType::Albedo, Info);
    ro->SetFlags(PolyFlags);
    renderContext->renderObjects.push_back(ro);
  }

  auto buffer = ro->AcquireBuffer<VertexPos3Norm3Tex0>();

  uint32_t primitiveCount = 0;
  FTransTexture* firstVertexInfo = Pts[0];

  const auto& md = (*renderContext->drawcallInfo->LastTextureMetadata);
  FVector vtx[3]{ firstVertexInfo->Point, {}, {} };
  FVector nrm[3]{ firstVertexInfo->Normal, {}, {} };
  for (int i = 2; i < NumPts; i++)
  {
    vtx[1] = Pts[i - 1]->Point;
    vtx[2] = Pts[i]->Point;
    nrm[1] = Pts[i - 1]->Normal;
    nrm[2] = Pts[i]->Normal;

    auto fixSigned0 = [](float& value) {
      if (value >= 0.0f && std::signbit(value)) { value = std::abs(value); }
      };
    auto d3dvtx0 = VertexPos3Norm3Tex0{ {vtx[0].X, vtx[0].Y, vtx[0].Z}, {nrm[0].X, nrm[0].Y, nrm[0].Z}, {md.multU * Pts[0]->U,			  md.multV * Pts[0]->V } };
    auto d3dvtx1 = VertexPos3Norm3Tex0{ {vtx[1].X, vtx[1].Y, vtx[1].Z}, {nrm[1].X, nrm[1].Y, nrm[1].Z}, {md.multU * Pts[i - 1]->U,  md.multV * Pts[i - 1]->V } };
    auto d3dvtx2 = VertexPos3Norm3Tex0{ {vtx[2].X, vtx[2].Y, vtx[2].Z}, {nrm[2].X, nrm[2].Y, nrm[2].Z}, { md.multU * Pts[i]->U,				md.multV * Pts[i]->V } };

    buffer->PushFace({
      { {vtx[0].X, vtx[0].Y, vtx[0].Z}, {nrm[0].X, nrm[0].Y, nrm[0].Z}, {md.multU * Pts[0]->U,			  md.multV * Pts[0]->V } },
      { {vtx[1].X, vtx[1].Y, vtx[1].Z}, {nrm[1].X, nrm[1].Y, nrm[1].Z}, {md.multU * Pts[i - 1]->U,    md.multV * Pts[i - 1]->V } },
      { {vtx[2].X, vtx[2].Y, vtx[2].Z}, {nrm[2].X, nrm[2].Y, nrm[2].Z}, { md.multU * Pts[i]->U,				md.multV * Pts[i]->V } }
    });
  }
}

void HighlevelRenderer::OnDrawMeshEnd(const FSceneNode* Frame, AActor* Actor)
{
  auto renderContext = g_ContextManager.GetContext();
  if (!renderContext->drawcallInfo)
  {
    //for now, skip any polygons not draw by DrawMesh
    return;
  }
  auto& callInfo = *renderContext->drawcallInfo;

  auto actor = Actor;
  auto parent = actor->Owner;
  const bool isPawn = actor->bIsPawn;
  const bool parentIsPawn = (parent && parent->bIsPawn);
  APawn* thisPawn = (isPawn ? Cast<APawn>(actor) : nullptr);
  APawn* parentPawn = (parentIsPawn ? Cast<APawn>(parent) : nullptr);
  const bool isWeapon = Cast<AWeapon>(actor);//(parentPawn ? parentPawn->Weapon == actor : false);
  const bool isPlayerWeapon = isWeapon && (parent == Frame->Viewport->Actor);

  auto wmCoords = GMath.UnitCoords;
  wmCoords = wmCoords * actor->Location * actor->Rotation;
  
  if (renderContext->drawcallInfo->SpecialCoords && isWeapon)
  {
    if (parent != nullptr && !isPlayerWeapon)
    {
      wmCoords = wmCoords * parent->Rotation;
    }
    wmCoords = wmCoords * renderContext->drawcallInfo->SpecialCoords->Inverse();
  }

  for (auto light = callInfo.LeafLights; light != nullptr; light = light->Next)
  {
    m_LightManager.AddFrameLight(light->Actor, &light->Location);
  }
  
  D3DXMATRIX wm;
  wm = UECoordsToMatrix(wmCoords);

  //The mesh was split up per texture+flag permutation. We have to render them all.
  for (auto& ro : renderContext->renderObjects)
  {
    callInfo.worldMatrix = wm;
    const auto flags = ro->GetFlags();
    const bool isEmissive = (callInfo.PolyFlags & PF_Unlit) != 0;
    for (int i = 0; i < (isEmissive ? 3 : 1); i++)
    {
      const auto meshIndex = actor->Mesh->GetIndex();
      const bool isFirstPerson = (parent == Frame->Viewport->Actor);
      const FrameContextManager::Context& capturedRenderContext = *renderContext;
      const auto& capturedFrame = *Frame;
      AddRenderCommand(isFirstPerson ? RenderCommandQueue::uiMesh : RenderCommandQueue::dynamicMesh, [=](const FrameContextManager::Context* pContext) {
        FTextureInfo* textureInfo = &ro->GetTextureInfo(RenderObjectTextureType::Albedo);
        if (i >= 1)
        {
          D3DXMATRIX scaledWorldMatrix;
          const FVector scale = (i == 1) ? FVector{ 0.9999f, 0.9999f, 0.9999f } : FVector{ 1.0001f, 1.0001f, 1.0001f };
          D3DXMATRIX s;
          D3DXMatrixScaling(&s, scale.X, scale.Y, scale.Z);
          D3DXMatrixMultiply(&scaledWorldMatrix, &wm, &s);
          SetWorldTransformState(scaledWorldMatrix);
          m_TextureManager.BindTexture(flags & ~PF_Unlit, m_TextureManager.ProcessTexture(flags & ~PF_Unlit, textureInfo));
        }
        else
        {
          SetWorldTransformState(wm);
          m_TextureManager.BindTexture(flags, m_TextureManager.ProcessTexture(flags, textureInfo));
        }
        m_LLRenderer->Render(ro.get());
        });
    }
  }
  renderContext->renderObjects.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HighlevelRenderer::PushUERenderObject(const void* pData, uint32_t pSize)
{
  if (pSize >= 4)
  {
    m_RenderObjectStack.push_back(std::make_pair(pSize, pData));
  }
}

void HighlevelRenderer::PopUERenderObject(uint32_t pSize)
{
  if (pSize >= 4)
  {
    check(pSize == m_RenderObjectStack.back().first);
    m_RenderObjectStack.pop_back();
  }
}

void HighlevelRenderer::AddRenderCommand(RenderCommandQueue pQueue, RenderCall&& pCB)
{
  m_CommandQueues[static_cast<uint32_t>(pQueue)].emplace_back(std::move(pCB));
}

void HighlevelRenderer::ExecuteCommandQueue(const FrameContextManager::Context* pContext, RenderCommandQueue pQueue)
{
  auto& queue = m_CommandQueues[static_cast<uint32_t>(pQueue)];
  for (auto& cmd : queue)
  {
    cmd(pContext);
  }
}

void HighlevelRenderer::ClearCommandQueue(RenderCommandQueue pQueue)
{
  auto& queue = m_CommandQueues[static_cast<uint32_t>(pQueue)];
  queue.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HighlevelRenderer::OnDrawUIBegin(const FSceneNode* Frame)
{
  g_ContextManager.PushFrameContext();
  auto ctx = g_ContextManager.GetContext();
  ctx->renderingUI = true;
  ctx->frameIsRasterized = true;
  m_LLRenderer->EmitDebugText(L"[EchelonRenderer] BeginUI");

#if 1
  {
    //Render any frame clipping (since it seems rtxremix doesn't do that for us).
    if (true)
    {
      m_LLRenderer->ClearDepth();

      UIntRect clipLeft, clipTop, clipRight, clipBottom;
      m_LLRenderer->GetClipRects(clipLeft, clipTop, clipRight, clipBottom);

      Draw2DScreenQuad(Frame, clipLeft.Left, clipLeft.Top, clipLeft.Width, clipLeft.Height, 0xFF000000ul);
      Draw2DScreenQuad(Frame, clipTop.Left, clipTop.Top, clipTop.Width, clipTop.Height, 0xFF000000ul);
      Draw2DScreenQuad(Frame, clipRight.Left, clipRight.Top, clipRight.Width, clipRight.Height, 0xFF000000ul);
      Draw2DScreenQuad(Frame, clipBottom.Left, clipBottom.Top, clipBottom.Width, clipBottom.Height, 0xFF000000ul);
      m_LLRenderer->ClearDepth();
    }
  }
#endif
}

void HighlevelRenderer::OnDrawUIEnd(const FSceneNode* Frame)
{

  auto ctx = g_ContextManager.GetContext();
  {
    //Finally; render the UI:
    check(Frame->Parent == nullptr);

    //Render UI meshes
    //ExecuteCommandQueue(RenderCommandQueue::ui);
  }
  ctx->renderingUI = false;
  g_ContextManager.PopFrameContext();

  m_LLRenderer->EmitDebugText(L"[EchelonRenderer] EndUI");
}

void HighlevelRenderer::OnDrawSprite(const FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags)
{
  auto& ctx = *g_ContextManager.GetContext();

  auto actorContainer = GetRenderObjectTopT<HActor>();
  if (actorContainer == nullptr)
  {
    return;
  }

  FDynamicSprite* sprite = [&]() -> FDynamicSprite* {
    for (FDynamicSprite* Sprite = Frame->Sprite; Sprite; Sprite = Sprite->RenderNext)
    {
      if (Sprite->Actor == actorContainer->Actor && Sprite->SpanBuffer == Span && Sprite->Z == pZ)
      {
        return Sprite;
      }
    }
    return nullptr;
    }();
  if (sprite == nullptr)
  {
    return;
  }

  const auto& texture = m_TextureManager.ProcessTexture(pPolyFlags, &TextureInfo);

  // Handle color processing
  FColor clampedColor = FColor(pColor);
  check(TextureInfo.MaxColor != nullptr);
  FColor maxColor = *TextureInfo.MaxColor;
  if (pPolyFlags & (PF_Modulated | PF_Masked)) {
    maxColor = FColor(0xFF, 0xFF, 0xFF, 0xFF);
  }
  D3DCOLOR Clr = (pPolyFlags & (PF_Modulated)) ?
    (maxColor.TrueColor() | 0xFF000000) :
    (FColor{
    Min(clampedColor.R, maxColor.R),
    Min(clampedColor.G, maxColor.G),
    Min(clampedColor.B, maxColor.B),
    Min(clampedColor.A, maxColor.A)
      }.TrueColor() | 0xFF000000);


  DWORD flags = (pPolyFlags & ~PF_Memorized) | TextureInfo.Texture->PolyFlags;
  if (TextureInfo.Palette && TextureInfo.Palette[128].A != 255 && !(pPolyFlags & PF_Translucent)) {
    flags |= PF_Highlighted;
  }

  D3DXVECTOR3 scaling{ TextureInfo.Texture->USize * actorContainer->Actor->DrawScale,
                      TextureInfo.Texture->VSize * actorContainer->Actor->DrawScale,
                      1.0f };
  D3DXVECTOR3 translation{ sprite->Location.X, sprite->Location.Y, sprite->Location.Z };

  D3DXQUATERNION rot{};
  D3DXMATRIX viewMatrix;
  this->GetViewMatrix(Frame->Coords, viewMatrix);
  D3DXMatrixInverse(&viewMatrix, NULL, &viewMatrix);
  D3DXQuaternionRotationMatrix(&rot, &viewMatrix);


#if 0
  D3DXVECTOR3 depthOffset{ 0.0f, 0.0f, 1.0f };
  constexpr float unrealSpriteZOffset = 32.0f;
  D3DXVec3TransformNormal(&depthOffset, &depthOffset, &viewMatrix);
  depthOffset *= unrealSpriteZOffset;
  translation += depthOffset;
#endif

  D3DXMATRIX wm;
  D3DXMatrixTransformation(&wm, nullptr, nullptr, &scaling, nullptr, &rot, &translation);

  auto [ro, roCreated] = m_RenderObjectManager.AcquireRenderObject<VertexPos4Color0Tex0>(Utils::CalculateKey(sprite), RenderObjectLifetime::Frame);
  if (roCreated)
  {
    ro->SetTexture(RenderObjectTextureType::Albedo, TextureInfo);
    ro->SetFlags(flags);
    ro->SetSceneNode(std::make_unique<FSceneNode>(*Frame));

    AddRenderCommand(RenderCommandQueue::pfx, [=](const FrameContextManager::Context* pContext){
      const auto flags = ro->GetFlags();
      auto textureHandle = m_TextureManager.ProcessTexture(flags, &ro->GetTextureInfo(RenderObjectTextureType::Albedo));
      m_TextureManager.BindTexture(flags, textureHandle);
      SetWorldTransformState(wm);
      m_LLRenderer->Render(ro.get());
    });
  }

  auto buffer = ro->AcquireBuffer<VertexPos4Color0Tex0>();
  buffer->PushFace({
    { {-0.5f, -0.5f, 0.0f, 1.0f }, Clr, {0.0f, 1.0f} },
    { {+0.5f, -0.5f, 0.0f, 1.0f }, Clr, {1.0f, 1.0f} },
    { {+0.5f, +0.5f, 0.0f, 1.0f }, Clr, {1.0f, 0.0f} },

    { {-0.5f, -0.5f, 0.0f, 1.0f }, Clr, {0.0f, 1.0f} },
    { {+0.5f, +0.5f, 0.0f, 1.0f }, Clr, {1.0f, 0.0f} },
    { {-0.5f, +0.5f, 0.0f, 1.0f }, Clr, {0.0f, 0.0f} }
   });
}

void HighlevelRenderer::OnDrawUI(const FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags)
{
  auto& ctx = *g_ContextManager.GetContext();

  DWORD flags = (pPolyFlags & ~PF_Memorized) | TextureInfo.Texture->PolyFlags;
  if (TextureInfo.Palette && TextureInfo.Palette[128].A != 255 && !(pPolyFlags & PF_Translucent)) {
    flags |= PF_Highlighted;
  }

  auto [ro,roCreated] = m_RenderObjectManager.AcquireRenderObject<PreTransformedVertexPos4Color0Tex0>(0, RenderObjectLifetime::Instant);
  auto vtex = ro->AcquireBuffer<PreTransformedVertexPos4Color0Tex0>();

  const float RZ = 1.0f / pZ;
  const float X1 = (pX + Frame->XB - 0.5);
  const float Y1 = (pY + Frame->YB - 0.5);
  const float	X2 = (X1 + pWidth);
  const float	Y2 = (Y1 + pHeight);

  const auto& md = m_TextureManager.ProcessTexture(flags, &TextureInfo)->md;
  const float RZUS = RZ * md.multU;
  const float U1 = (pTexCoordU)*RZUS;
  const float U2 = (pTexCoordU + pTexCoordUL) * RZUS;
  const float RZVS = RZ * md.multV;
  const float V1 = (pTexCoordV)*RZVS;
  const float V2 = (pTexCoordV + pTexCoordVL) * RZVS;

  // Handle color processing
  FColor clampedColor = FColor(pColor);
  check(TextureInfo.MaxColor != nullptr);
  FColor maxColor = *TextureInfo.MaxColor;

  if (flags & (PF_Modulated | PF_Masked)) {
    maxColor = FColor(0xFF, 0xFF, 0xFF, 0xFF);
  }

  D3DCOLOR Clr = (flags & (PF_Modulated)) ?
               (maxColor.TrueColor() | 0xFF000000) :
               (FColor{
                   Min(clampedColor.R, maxColor.R),
                   Min(clampedColor.G, maxColor.G),
                   Min(clampedColor.B, maxColor.B),
                   Min(clampedColor.A, maxColor.A)
               }.TrueColor() | 0xFF000000);

  vtex->PushFace({
      { {X1, Y1, pZ, 1.0f}, Clr, {U1, V1} },
      { {X2, Y1, pZ, 1.0f}, Clr, {U2, V1} },
      { {X2, Y2, pZ, 1.0f}, Clr, {U2, V2} },
    
      { {X1, Y1, pZ, 1.0f}, Clr, {U1, V1} },
      { {X2, Y2, pZ, 1.0f}, Clr, {U2, V2} },
      { {X1, Y2, pZ, 1.0f}, Clr, {U1, V2	} },
    });

  if (roCreated)
  {
    ro->SetFlags(flags);
    ro->SetTexture(RenderObjectTextureType::Albedo, TextureInfo);
    ro->SetSceneNode(std::make_unique<FSceneNode>(*Frame));

    AddRenderCommand(RenderCommandQueue::ui, [=](const FrameContextManager::Context* pContext){
      const auto flags = ro->GetFlags();
      auto textureHandle = m_TextureManager.ProcessTexture(flags, &ro->GetTextureInfo(RenderObjectTextureType::Albedo));
      m_TextureManager.BindTexture(flags, textureHandle);
      m_LLRenderer->Render(ro.get());
    });
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HighlevelRenderer::SetWorldTransformStateToIdentity()
{
  static D3DXMATRIX identity = []() { D3DXMATRIX m; D3DXMatrixIdentity(&m); return m; }();
  m_LLRenderer->SetWorldMatrix(identity);
}

void HighlevelRenderer::SetWorldTransformState(const D3DXMATRIX& pMatrix)
{
  m_LLRenderer->SetWorldMatrix(pMatrix);
}


void HighlevelRenderer::SetViewState(const FSceneNode* Frame, ViewType viewType)
{
  auto& ctx = *g_ContextManager.GetContext();

  D3DXMATRIX vm;
  if (viewType == ViewType::game)
  {
    GetViewMatrix(Frame->Coords, vm);
  }
  if (viewType == ViewType::engine)
  {
    D3DXMatrixIdentity(&vm);
    vm(0, 0) = 1.0f;
    vm(0, 1) = 0.0f;
    vm(0, 2) = 0.0f;

    vm(1, 0) = 0.0f;
    vm(1, 1) = -1.0f;
    vm(1, 2) = 0.0f;

    vm(2, 0) = 0.0f;
    vm(2, 1) = 0.0f;
    vm(2, 2) = -1.0f;
  }
  else if (viewType == ViewType::identity)
  {
    D3DXMatrixIdentity(&vm);
  }
  m_LLRenderer->SetViewMatrix(vm);

  if (ctx.overrides.enableViewportXYOffsetWorkaround)
  {
    auto sz = m_LLRenderer->GetDisplaySurfaceSize();
    m_LLRenderer->SetViewport(0, 0, sz.first, sz.second);
  }
  else
  {
    m_LLRenderer->SetViewport(Frame->XB, Frame->YB, Frame->X, Frame->Y);
  }
}

void HighlevelRenderer::SetProjectionState(const FSceneNode* Frame, ProjectionType projection) {
  D3DXMATRIX d3dProj;
  if (projection == ProjectionType::perspective)
  {
    GetPerspectiveProjectionMatrix(Frame, d3dProj);
    m_LLRenderer->SetProjectionMatrix(d3dProj);
  }
  else if (projection == ProjectionType::uiorthogonal)
  {
    D3DXMatrixOrthoLH(&d3dProj, Frame->FX, Frame->FY, 0.0001f, 2.0f);
    /*c  r*/
    d3dProj.m[3][0] = -1.0f;
    d3dProj.m[3][1] = +1.0f;
    d3dProj.m[1][1] *= -1.0f;
    d3dProj.m[3][3] = 1.0f;
    m_LLRenderer->SetProjectionMatrix(d3dProj);
  }
  else if (projection == ProjectionType::orthogonal)
  {
    assert(false); //iimplementation probably needs to be fixed due to the render range changes.

    const float fovangle = Frame->Viewport->Actor->FovAngle;
    const float aspect = (Frame->FY / Frame->FX);
    const float fovHalfAngle = appTan(fovangle * (PI / 360.0f)) * RenderRanges::Engine.NearRange;

    float viewportW = float(Frame->Viewport->SizeX);
    float viewportH = float(Frame->Viewport->SizeY);
    D3DXMatrixOrthoOffCenterRH(&d3dProj, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, RenderRanges::Engine.FarRange);

    m_LLRenderer->SetProjectionMatrix(d3dProj);
  }
  else if (projection == ProjectionType::identity)
  {
    D3DXMatrixIdentity(&d3dProj);
    m_LLRenderer->SetProjectionMatrix(d3dProj);
  }
}