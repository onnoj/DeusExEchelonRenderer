#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <chrono>
#include <dxgi.h>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <xxhash.h>

#include "rendering/llrenderer.h"
#include "rendering/scenemanager.h"
#include "utils/debugmenu.h"
#include "utils/utils.h"
#include "utils/materialdebugger.h"

bool LowlevelRenderer::Initialize(HWND hWnd, uint32_t pWidth, uint32_t pHeight, uint32_t pColorBytes, bool pFullscreen)
{
  check(IsWindow(hWnd));
  GLog->Log(L"[EchelonRenderer]\t Initializing LLRenderer");
  HRESULT hr{};
  if (m_API == nullptr)
  {
    GLog->Log(L"[EchelonRenderer]\t Initializing Direct3D 9 API");
    m_API = Direct3DCreate9(D3D_SDK_VERSION);
    check(m_API != nullptr);
    if (m_API == nullptr)
    {
      GWarn->Logf(L"[EchelonRenderer-WARN]\t Failed to initialize D3D9/Remix API; last error code was %08x", GetLastError());
      return false;
    }
  }
  else
  {
    GLog->Log(L"[EchelonRenderer]\t Already had api, didn't (re)initalize.");
  }

  m_outputSurface.colorBytes = pColorBytes;
  //m_outputSurface.hwnd = hWnd;
  if (pFullscreen)
  {
    if (auto res = FindClosestResolution(pWidth, pHeight); res)
    {
      GLog->Logf(L"[EchelonRenderer]\t Fullscreen was requested, matched %dx%d to available resolution %dx%d.", pWidth, pHeight, res->Width, res->Height);
      pWidth = res->Width;
      pHeight = res->Height;
    }
    else
    {
      GLog->Logf(L"[EchelonRenderer]\t could not find matching fullscreen resolution for %dx%d, possible device returned no modes. Swithing to windowed.", pWidth, pHeight);
      pFullscreen = false;
    }
  }

  ResizeDisplaySurface(0, 0, pWidth, pHeight, pFullscreen);

  D3DFORMAT depthFormat = [&]() {
    constexpr D3DFORMAT depthFormats[] = {
      D3DFMT_D32,
      D3DFMT_D32F_LOCKABLE,
      D3DFMT_D32_LOCKABLE,
      D3DFMT_D24X8,
      D3DFMT_D24S8,
      D3DFMT_D24X4S4,
      D3DFMT_D24FS8,
      D3DFMT_D16,
      D3DFMT_D16_LOCKABLE,
      D3DFMT_D15S1,
      D3DFMT_S8_LOCKABLE,
    };

    D3DDISPLAYMODE currentMode{0};
    auto hResult = m_API->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &currentMode);
    if (SUCCEEDED(hResult))
    {
      for (auto fmt : depthFormats)
      {
        if (SUCCEEDED(m_API->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, currentMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, fmt)))
        {
          return fmt;
        }
      }
    }
    return D3DFMT_D16;
    }();

  D3DPRESENT_PARAMETERS d3dpp{0};
  d3dpp.Windowed = !pFullscreen;
  d3dpp.hDeviceWindow = hWnd;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = (pFullscreen ? D3DFMT_X8R8G8B8 : D3DFMT_UNKNOWN);
  d3dpp.BackBufferWidth = m_outputSurface.width;
  d3dpp.BackBufferHeight = m_outputSurface.height;
  d3dpp.BackBufferCount = 1;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = depthFormat;
  d3dpp.Flags = 0;// D3DPRESENT_DONOTWAIT;//D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
  d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT; // D3DPRESENT_INTERVAL_IMMEDIATE;
  d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

  GLog->Logf(L"D3DPRESENT_PARAMETERS: fullscreen:%d w:%d h:%d", pFullscreen ? 1 : 0, d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);
  if (m_Device == nullptr)
  {
    hr = m_API->CreateDevice(
      D3DADAPTER_DEFAULT,
      D3DDEVTYPE_HAL,
      hWnd,
      D3DCREATE_HARDWARE_VERTEXPROCESSING,
      &d3dpp,
      &m_Device);
    GLog->Log(SUCCEEDED(hr) ? L"[EchelonRenderer]\t Initialized Direct3D 9 device" : L"[EchelonRenderer]\t Failed to initialize D3D9 device");
  }
  else
  {
    hr = m_Device->Reset(&d3dpp);
    GLog->Log(SUCCEEDED(hr) ? L"[EchelonRenderer]\t Reset Direct3D 9 device" : L"[EchelonRenderer]\t Failed to reset D3D9 device");
  }

  if (!SUCCEEDED(hr))
  {
    GWarn->Logf(L"[EchelonRenderer-WARN]\t Error code was: %08x", hr);
    GLog->Logf(L"d3dpp.Windowed = %d", d3dpp.Windowed);
    GLog->Logf(L"d3dpp.hDeviceWindow = 0x%x", d3dpp.hDeviceWindow);
    GLog->Logf(L"d3dpp.SwapEffect = %d", d3dpp.SwapEffect);
    GLog->Logf(L"d3dpp.BackBufferFormat = %d", d3dpp.BackBufferFormat);
    GLog->Logf(L"d3dpp.BackBufferWidth = %d", d3dpp.BackBufferWidth);
    GLog->Logf(L"d3dpp.BackBufferHeight = %d", d3dpp.BackBufferHeight);
    GLog->Logf(L"d3dpp.BackBufferCount = %d", d3dpp.BackBufferCount);
    GLog->Logf(L"d3dpp.EnableAutoDepthStencil = %d", d3dpp.EnableAutoDepthStencil);
    GLog->Logf(L"d3dpp.AutoDepthStencilFormat = %d", d3dpp.AutoDepthStencilFormat);
    GLog->Logf(L"d3dpp.Flags = %x", d3dpp.Flags);
    GLog->Logf(L"d3dpp.PresentationInterval = %d", d3dpp.PresentationInterval);
    GLog->Logf(L"d3dpp.FullScreen_RefreshRateInHz = %d", d3dpp.FullScreen_RefreshRateInHz);

    if (hr == D3DERR_DEVICELOST)
    {
      GWarn->Log(L"[EchelonRenderer-WARN]\t Device lost while initializing or resetting device, retrying...");
      return false;
    }
    if (hr == D3DERR_NOTAVAILABLE)
    {
      GWarn->Log(L"[EchelonRenderer-WARN]\t Device does not support the requested parameters.");
      return false;
    }
    if (hr == D3DERR_INVALIDCALL)
    {
      GWarn->Log(L"[EchelonRenderer-WARN]\t Device was passed invalid parameters.");
      return false;
    }

    GError->Logf(L"Unrecoverable error when (re)initializing the d3d9 device.");
    return false;
  }
  check(SUCCEEDED(hr));

  hr = m_Device->GetDeviceCaps(&m_caps);
  if (FAILED(hr))
  {
    GWarn->Logf(L"[EchelonRenderer-WARN]\t Unable to pull device caps.  Error code was: %08x", hr);
    return false;
  }

  hr = m_Device->CreateDepthStencilSurface(
    m_outputSurface.width, // Width of the depth buffer surface
    m_outputSurface.height, // Height of the depth buffer surface
    depthFormat, // Depth buffer format (should match the one specified in d3dpp)
    D3DMULTISAMPLE_NONE, // Multisample type
    0, // Multisample quality
    FALSE, // Discard stencil data (if any)
    &m_DepthStencilSurface,
    nullptr
  );
  if (!SUCCEEDED(hr))
  {
    GWarn->Logf(L"[EchelonRenderer-WARN]\t Error code was: %08x", hr);
    GLog->Logf(L"m_outputSurface.Width = %d", m_outputSurface.width);
    GLog->Logf(L"m_outputSurface.height = %d", m_outputSurface.height);

    if (hr == D3DERR_DEVICELOST)
    {
      GWarn->Log(L"[EchelonRenderer-WARN]\t Device lost while creating DepthStencilSurface, retrying...");
      return false;
    }

    GError->Logf(L"Unrecoverable error when creating the Depth Stencil buffer.");
    return false;
  }
  check(SUCCEEDED(hr));

  if (m_fakeLightBuffer == nullptr)
  {
    auto hr = m_Device->CreateVertexBuffer(
      3 * sizeof(VertexPos3Tex0),
      0,
      D3DFVF_XYZ,
      D3DPOOL_MANAGED,
      &m_fakeLightBuffer,
      nullptr
    );
    m_vtxBufferAllocations++;
    check(SUCCEEDED(hr));
    float* xyzw = nullptr;
    m_fakeLightBuffer->Lock(0, sizeof(float) * 3 * 3, reinterpret_cast<void**>(&xyzw), 0);
    for (int i = 0; i < 3; i++)
    {
      xyzw[(i * 3) + 0] = 0.0f;
      xyzw[(i * 3) + 1] = 0.0f;
      xyzw[(i * 3) + 2] = 0.0f;
    }
    m_fakeLightBuffer->Unlock();
  }

  ResizeDisplaySurface(0, 0, m_outputSurface.width, m_outputSurface.height, m_outputSurface.fullscreen);
  InitializeDeviceState();
  return 1;
}

void LowlevelRenderer::Shutdown()
{
  //Don't actually destroy these resources.. we'll just reset the device..
  for (auto& bg : m_bufferedGeo)
  {
    bg.second.resourceAgeTracker->erase(bg.first);
    bg.second.buffer->Release();
    bg.second.buffer = nullptr;
  }
  m_bufferedGeo.clear();
  for (auto& rm : m_OldResources)
  {
    for (auto& r : rm)
    {
      static_cast<IDirect3DVertexBuffer9*>(r.second)->Release();
    }
    rm.clear();
  }

  if (m_fakeLightBuffer)
  {
    m_fakeLightBuffer->Release();
    m_fakeLightBuffer = nullptr;
  }

  if (m_DepthStencilSurface)
  {
    m_DepthStencilSurface->Release();
    m_DepthStencilSurface = nullptr;
  }

#if 0
  if (m_Device)
  {
    m_Device->Release();
    m_Device = nullptr;
  }

  if (m_API)
  {
    m_API->Release();
    m_API = nullptr;
  }
#endif

  GLog->Log(L"[EchelonRenderer]\t LLRenderer shutdown");
  ::Sleep(100);
}

void LowlevelRenderer::BeginScene()
{
  check(m_IsInFrame);
  check(m_IsInScene == 0);
  m_IsInScene++;

  HRESULT res{};
  res = m_Device->BeginScene();
   check(SUCCEEDED(res));
  g_Stats.Writer().BeginScene();
}

void LowlevelRenderer::EndScene()
{
  auto res = m_Device->EndScene();
  check(SUCCEEDED(res));
  g_Stats.Writer().EndScene();
  m_IsInScene--;
  check(m_IsInScene == 0);
}

void LowlevelRenderer::RenderTriangleListBuffer(DWORD pFVF, const void* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pVertexSize, const uint32_t pHash, const uint32_t pDebug)
{
  auto& ctx = *g_ContextManager.GetContext();

  g_SceneManager.Validate();
  g_Stats.Writer().DrawCall();

  IDirect3DVertexBuffer9* buffer = nullptr;
  auto bufferedGeoIterator = (pHash != 0 ? m_bufferedGeo.find(pHash) : m_bufferedGeo.end());
  if (bufferedGeoIterator != m_bufferedGeo.end())
  {
    buffer = (*bufferedGeoIterator).second.buffer;
  }

  static bool debug = false;
  if (debug && buffer == nullptr)
  {
    return;
  }

  if (buffer == nullptr)
  {
    auto hr = m_Device->CreateVertexBuffer(
      pVertexCount * pVertexSize,
      0,
      pFVF,
      D3DPOOL_MANAGED,
      &buffer,
      nullptr
    );
    if (!SUCCEEDED(hr))
    {
      GWarn->Logf(L"[EchelonRenderer-WARN]\t D3D failed to create vertex buffer, error 0x%08x", hr);
      return;
    }
    m_vtxBufferAllocations++;

    if (pHash != 0)
    {
      m_bufferedGeo[pHash] = { buffer, m_FrameOldResources };
    }
    m_FrameOldResources->insert(std::make_pair(pHash, buffer));

    void* recv = nullptr;
    buffer->Lock(0, pVertexCount * pVertexSize, reinterpret_cast<void**>(&recv), 0);
    {
      memcpy(recv, pVertices, pVertexCount * pVertexSize);
    }
    buffer->Unlock();
  }
  else
  {
    auto& bufferedGeoValue = (*bufferedGeoIterator).second;
    if (pHash != 0 && bufferedGeoValue.resourceAgeTracker != m_FrameOldResources)
    {
      m_FrameOldResources->insert(std::make_pair(pHash, buffer));
      bufferedGeoValue.resourceAgeTracker->erase(pHash);
      bufferedGeoValue.resourceAgeTracker = m_FrameOldResources;
    }
  }

  CheckDirtyMatrices();

#if EE_DEBUG
  if (pDebug != 0)
  {
    wchar_t b[256]{ 0 };
    swprintf_s(b, L"Dbg: %08u", pDebug);
    D3DPERF_SetMarker(0x00, &b[0]);
  }
#endif

  RenderStateDebugger::Process(this, pDebug);
  TextureStageStateDebugger::Process(this, pDebug);

  //Commit primitive
  {
    HRESULT res = S_OK;
    res = m_Device->SetStreamSource(0, buffer, 0, pVertexSize); check(SUCCEEDED(res));
    res = m_Device->SetIndices(nullptr); check(SUCCEEDED(res));
    //m_Device->SetVertexShaderConstantI(pDebug, nullptr, 0);
    res = m_Device->SetFVF(pFVF); check(SUCCEEDED(res));
    res = m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, primitiveCount);
    check(SUCCEEDED(res));
  }
}

void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos3Tex0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos3Tex0),
    pHash,
    pDebug
  );
}

void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos3Tex0Tex1* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 | D3DFVF_TEX2 /*| D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5 */,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos3Tex0Tex1),
    pHash,
    pDebug
  );
}


void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos3Norm3Tex0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZ | D3DFVF_NORMAL | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos3Norm3Tex0),
    pHash,
    pDebug
  );
}

void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos3Tex0to4* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos3Tex0to4),
    pHash,
    pDebug
  );
}

void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos4Color0Tex0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos4Color0Tex0),
    pHash,
    pDebug
  );
}

void LowlevelRenderer::RenderTriangleList(const LowlevelRenderer::VertexPos3Color0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug)
{
  return RenderTriangleListBuffer(
    D3DFVF_XYZ | D3DFVF_DIFFUSE /*| D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/,
    pVertices,
    primitiveCount,
    pVertexCount,
    sizeof(LowlevelRenderer::VertexPos3Color0),
    pHash,
    pDebug
  );
}

void LowlevelRenderer::DisableLight(int32_t index)
{
  if (!g_options.hasLights)
  {
    return;
  }
  g_SceneManager.Validate();
  m_Device->LightEnable(index, FALSE);
}

void LowlevelRenderer::EmitDebugText(const wchar_t* pTxt)
{
#if EE_DEBUG
  D3DPERF_SetMarker(0x00, pTxt);
#endif
}

void LowlevelRenderer::RenderLight(int32_t index, const D3DLIGHT9& pLight)
{
  if (!g_options.hasLights)
  {
    return;
  }
  g_SceneManager.Validate();
  CheckDirtyMatrices();

  static const int maxLightsPerCall = [&]() {
    D3DCAPS9 caps{};
    m_Device->GetDeviceCaps(&caps);
    return (caps.MaxActiveLights > 0) ? caps.MaxActiveLights : 8;
    }();;
    static int idx = 0;
    idx %= maxLightsPerCall;

    m_CanFlushLights = true;

    m_Device->LightEnable(index, TRUE);
    m_Device->SetLight(index, &pLight);

    if (idx == maxLightsPerCall - 1)
    {
      FlushLights();
    }
    idx++;
}

void LowlevelRenderer::FlushLights()
{
  if (!g_options.hasLights)
  {
    return;
  }

  if (m_CanFlushLights)
  {
    g_SceneManager.Validate();
    CheckDirtyMatrices();

    HRESULT res = S_OK;
    res = m_Device->SetStreamSource(0, m_fakeLightBuffer, 0, sizeof(float) * 3 * 3); check(SUCCEEDED(res));
    res = m_Device->SetIndices(nullptr); check(SUCCEEDED(res));
    res = m_Device->SetFVF(D3DFVF_XYZ); check(SUCCEEDED(res));
    res = m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    check(SUCCEEDED(res));
    m_CanFlushLights = false;
  }
}



void LowlevelRenderer::BeginFrame()
{
  g_SceneManager.Validate();
  g_Stats.Writer().BeginFrame();

  g_ContextManager.PushFrameContext();
  auto& ctx = *g_ContextManager.GetContext();
  ctx.frameIsRasterized |= !Misc::IsNvRemixAttached(false);
  //validate if the device is still valid
  //DWORD numPasses = 0;
  //if (m_Device->ValidateDevice(&numPasses) != D3D_OK)
  //{
  //	m_IsInFrame = false;
  //	Shutdown();
  //	Initialize(m_outputSurface.hwnd, m_outputSurface.width, m_outputSurface.height, m_outputSurface.colorBytes, m_outputSurface.fullscreen);
  //	return;
  //}

  //clean up
  m_IsInFrame = true;
  static auto lastSeconds = 0.0f;
  float now = appSeconds();
  //if (now - lastSeconds > 5.0f)
  {
    lastSeconds = now;

    m_FrameOldResources = &m_OldResources[m_FrameOldResourcesIdx];
    m_FrameOldResourcesIdx = (m_FrameOldResourcesIdx + 1) % (sizeof(m_OldResources) / sizeof(m_OldResources[0]));

    //Clean the upcoming bucket:
    for (auto r : m_OldResources[m_FrameOldResourcesIdx])
    {
      auto newRetCount = static_cast<IDirect3DVertexBuffer9*>(r.second)->Release();
      assert(newRetCount == 0);
      m_bufferedGeo.erase(r.first);
      m_vtxBufferAllocations--;
    }
    m_OldResources[m_FrameOldResourcesIdx].clear();
  }

  //Setup buffers
  m_Device->SetDepthStencilSurface(m_DepthStencilSurface);

  //Setup scene
  {
    HRESULT res = S_OK;
    
    res = this->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    res = this->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    res = this->SetRenderState(D3DRS_COLORWRITEENABLE, TRUE);
    res = this->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    
    res = this->SetRenderState(D3DRS_SPECULARENABLE, TRUE);    
    res = this->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    res = this->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    res = this->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    res = this->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    res = this->SetRenderState(D3DRS_ALPHAREF, 127);
    res = this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    res = this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    res = this->SetRenderState(D3DRS_LIGHTING, FALSE); check(SUCCEEDED(res));
    res = this->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); check(SUCCEEDED(res));
    res = this->SetRenderState(D3DRS_DITHERENABLE, TRUE);
    res = this->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    bool hasWireframe = false;
    g_DebugMenu.DebugVar("Rendering", "Wireframe", DebugMenuUniqueID(), hasWireframe);
    res = this->SetRenderState(D3DRS_FILLMODE, hasWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID); check(SUCCEEDED(res));

    res = this->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    res = this->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    res = this->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 1);

    this->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    this->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    this->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_DIFFUSE);
    this->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    //TODO: Document why are we doing this?
    if (!m_CurrentState->m_WorldMatrix)
    {
      D3DXMATRIX m; D3DXMatrixIdentity(&m);
      m_CurrentState->m_WorldMatrix = m;
    }
  }

  //Start rendering
  //auto res = m_Device->BeginScene();	check(SUCCEEDED(res));
  ClearDisplaySurface(Vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
}

void LowlevelRenderer::EndFrame()
{
  g_SceneManager.Validate();
  if (!m_IsInFrame)
  {
    return;
  }

  static auto lastTime = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  int timeSinceLastTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>((now - lastTime)).count();
  lastTime = now;
  if (m_Device == nullptr) return;
  //auto res = m_Device->EndScene(); check(SUCCEEDED(res));
  CheckDirtyMatrices();

#if 1
  auto res = m_Device->Present(NULL, NULL, NULL, NULL);
#else
  HRESULT res = D3D_OK;
  static std::mutex presentMutex;
  static std::condition_variable canPresentCV;
  static bool canPresent = false;
  static bool hasPresented = true;
  static std::thread presentThread([&]() {
    while (true)
    {
      std::unique_lock lk(presentMutex);
      canPresentCV.wait(lk, [] 
        { 
          return canPresent; 
        });

      while (m_Device->Present(NULL, NULL, NULL, NULL) == D3DERR_WASSTILLDRAWING)
      {
      }
      canPresent = false;
      hasPresented = true;
      lk.unlock();
      canPresentCV.notify_one();
    }
    });
  static bool isThreadRunning = [&]() { presentThread.detach(); return true; }();

  //Wait for the present thread to be done if it was still running
  {
    std::unique_lock lk(presentMutex);
    canPresentCV.wait(lk, [] 
      { 
        return hasPresented; 
      });
  }

  //Trigger a present
  {
    std::unique_lock lk(presentMutex);
    canPresent = true;
    canPresentCV.notify_one();
  }
#endif

  //Debug
  {
    auto cpy = m_vtxBufferAllocations;
    g_DebugMenu.DebugVar("Rendering", "VTX Buffers", DebugMenuUniqueID(), cpy);
  }
  if (res == D3DERR_DEVICELOST)
  {
    m_IsInFrame = false;
    Shutdown();
    Initialize((HWND)GRenderDevice->Viewport->GetWindow(), m_outputSurface.width, m_outputSurface.height, m_outputSurface.colorBytes, m_outputSurface.fullscreen);
    return;
  }
  check(SUCCEEDED(res));
  m_IsInFrame = false;
  g_Stats.Writer().EndFrame();
  g_ContextManager.PopFrameContext();
}

/**
Set resolution and windowed/fullscreen.
*/
bool LowlevelRenderer::ResizeDisplaySurface(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight, bool pFullScreen)
{
  assert(pWidth != 0 && pHeight != 0);
  bool surfaceChanged = (m_outputSurface.width != pWidth) || (m_outputSurface.height != pHeight);
  m_outputSurface.width = pWidth;
  m_outputSurface.height = pHeight;
  m_outputSurface.fullscreen = pFullScreen;
  if (surfaceChanged && m_Device != nullptr)
  {
    GWarn->Log(L"Render surface has changed, we need to shutdown and reinitialize the renderer.");
    Shutdown();
    Initialize((HWND)GRenderDevice->Viewport->GetWindow(), pWidth, pHeight, m_outputSurface.colorBytes, pFullScreen);
  }
  if (m_Device)
  {
    return SetViewport(pWidth, pHeight, pWidth, pHeight);
  }
  return false;
}

void LowlevelRenderer::CheckDirtyMatrices()
{
  if (m_CurrentState->m_WorldMatrixPending)
  {
    auto& m = *(m_CurrentState->m_WorldMatrixPending);
    auto result = m_Device->SetTransform(D3DTS_WORLD, &m);
    check(SUCCEEDED(result));
    m_CurrentState->m_WorldMatrix = m_CurrentState->m_WorldMatrixPending;
    m_CurrentState->m_WorldMatrixPending.reset();
  }

  if (m_CurrentState->m_ViewMatrixPending)
  {
    auto& m = *(m_CurrentState->m_ViewMatrixPending);

    D3DXMATRIX oldMInv, mInv;
    D3DXMATRIX oldM;
    m_Device->GetTransform(D3DTS_VIEW, &oldM);
    D3DXMatrixInverse(&oldMInv, nullptr, &oldM);
    D3DXMatrixInverse(&mInv, nullptr, &(D3DXMATRIX(m)));
    static D3DXVECTOR3 origin{ 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 posA, posB;
    D3DXVec3TransformCoord(&posA, &origin, &oldMInv);
    D3DXVec3TransformCoord(&posB, &origin, &mInv);
    D3DXVECTOR3 diff;
    D3DXVec3Subtract(&diff, &posA, &posB);
    float l = D3DXVec3Length(&diff);

    {
      D3DXMATRIX newM = D3DXMATRIX(m);
      D3DXVECTOR3 mAxis[] = { {newM(0,0), newM(0,1), newM(0,2)}, {newM(1,0), newM(1,1), newM(1,2)}, {newM(2,0), newM(2,1), newM(2,2)} };
      float xy = fabsf(D3DXVec3Dot(&mAxis[0], &mAxis[1]));
      float xz = fabsf(D3DXVec3Dot(&mAxis[0], &mAxis[2]));
      float yz = fabsf(D3DXVec3Dot(&mAxis[1], &mAxis[2]));
      check(xy < 0.001f && xz < 0.001f && yz < 0.001f);
      check(newM(3, 3) == 1.0f);
    }
    auto result = m_Device->SetTransform(D3DTS_VIEW, &m);
    check(SUCCEEDED(result));

    m_CurrentState->m_ViewMatrix = m_CurrentState->m_ViewMatrixPending;
    m_CurrentState->m_ViewMatrixPending.reset();
  }
  else
  {
    auto& m = *(m_CurrentState->m_ViewMatrix);

    //debugging:
    D3DXMATRIX vm;
    m_Device->GetTransform(D3DTS_VIEW, &vm);
    auto diff = memcmp(&vm, &m, sizeof(vm));
    check(diff == 0);
  }

  if (m_CurrentState->m_ProjectionMatrixPending)
  {
    auto& m = *(m_CurrentState->m_ProjectionMatrixPending);
    auto result = m_Device->SetTransform(D3DTS_PROJECTION, &m);
    check(SUCCEEDED(result));

    m_CurrentState->m_ProjectionMatrix = m_CurrentState->m_ProjectionMatrixPending;
    m_CurrentState->m_ProjectionMatrixPending.reset();
  }

  check(m_CurrentState->m_WorldMatrix);
  check(m_CurrentState->m_ViewMatrix);
  check(m_CurrentState->m_ProjectionMatrix);
}

void LowlevelRenderer::SetWorldMatrix(const D3DMATRIX& pMatrix)
{
  m_CurrentState->m_WorldMatrixPending = pMatrix;
}

void LowlevelRenderer::SetViewMatrix(const D3DMATRIX& pMatrix)
{
  m_CurrentState->m_ViewMatrixPending = pMatrix;
}

void LowlevelRenderer::SetProjectionMatrix(const D3DMATRIX& pMatrix)
{
  m_CurrentState->m_ProjectionMatrixPending = pMatrix;
}

bool LowlevelRenderer::SetViewport(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight)
{
  m_DesiredViewportLeft = pLeft;
  m_DesiredViewportTop = pTop;
  m_DesiredViewportWidth = pWidth;
  m_DesiredViewportHeight = pHeight;

  if (!m_DesiredViewportMinZ)
  {
    m_DesiredViewportMinZ = RenderRanges::Engine.DepthMin;
  }

  if (!m_DesiredViewportMaxZ)
  {
    m_DesiredViewportMaxZ = RenderRanges::Engine.DepthMax;
  }
  return ValidateViewport();
}

void LowlevelRenderer::GetViewport(uint32_t& pmLeft, uint32_t& pmTop, uint32_t& pmWidth, uint32_t& pmHeight)
{
  pmLeft = (m_CurrentState->m_ViewportLeft ? *m_CurrentState->m_ViewportLeft : 0);
  pmTop = (m_CurrentState->m_ViewportTop ? *m_CurrentState->m_ViewportTop : 0);
  pmWidth = (m_CurrentState->m_ViewportLeft ? *m_CurrentState->m_ViewportWidth : 0);
  pmHeight = (m_CurrentState->m_ViewportLeft ? *m_CurrentState->m_ViewportHeight : 0);
}

void LowlevelRenderer::GetClipRects(UIntRect& pmLeft, UIntRect& pmTop, UIntRect& pmRight, UIntRect& pmBottom)
{
  uint32_t viewportL, viewportT, viewportW, viewportH;
  GetViewport(viewportL, viewportT, viewportW, viewportH);

  pmLeft = UIntRect(0, 0, viewportL, m_outputSurface.height);
  pmTop = UIntRect(0, 0, m_outputSurface.width, viewportT);
  pmRight = UIntRect(viewportL + viewportW, 0, m_outputSurface.width - (viewportL + viewportW), m_outputSurface.height);
  pmBottom = UIntRect(0, viewportT + viewportH, m_outputSurface.width, m_outputSurface.height - (viewportT + viewportH));
}

void LowlevelRenderer::SetViewportDepth(const RangeDefinition& pType)
{
  SetViewportDepth(pType.DepthMin, pType.DepthMax);
}

void LowlevelRenderer::SetViewportDepth(float pMinZ, float pMaxZ)
{
  m_DesiredViewportMinZ = pMinZ;
  m_DesiredViewportMaxZ = pMaxZ;
  ValidateViewport();
}

void LowlevelRenderer::ResetViewportDepth()
{
  SetViewportDepth(RenderRanges::Engine);
  ValidateViewport();
}

bool LowlevelRenderer::ValidateViewport()
{
  if(m_DesiredViewportTop) g_DebugMenu.DebugVar("Rendering", "Viewport.Top", DebugMenuUniqueID(), *m_DesiredViewportTop, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 1080 });
  if(m_DesiredViewportLeft)  g_DebugMenu.DebugVar("Rendering", "Viewport.Left", DebugMenuUniqueID(), *m_DesiredViewportLeft, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 1080 });
  if(m_DesiredViewportWidth) g_DebugMenu.DebugVar("Rendering", "Viewport.Width", DebugMenuUniqueID(), *m_DesiredViewportWidth, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4096 });
  if(m_DesiredViewportHeight) g_DebugMenu.DebugVar("Rendering", "Viewport.Height", DebugMenuUniqueID(), *m_DesiredViewportHeight, { DebugMenuValueOptions::editor::slider, 0.0f, 0.0f, 0, 4096 });

  if ((m_DesiredViewportLeft && (m_CurrentState->m_ViewportLeft != *m_DesiredViewportLeft)) ||
    (m_DesiredViewportWidth && (m_CurrentState->m_ViewportWidth != *m_DesiredViewportWidth)) ||
    (m_DesiredViewportTop && (m_CurrentState->m_ViewportLeft != *m_DesiredViewportTop)) ||
    (m_DesiredViewportHeight && (m_CurrentState->m_ViewportHeight != *m_DesiredViewportHeight)) ||
    (m_DesiredViewportMinZ && (m_CurrentState->m_ViewportMinZ != *m_DesiredViewportMinZ)) ||
    (m_DesiredViewportMaxZ && (m_CurrentState->m_ViewportMaxZ != *m_DesiredViewportMaxZ))
    )
  {
    m_CurrentState->m_ViewportLeft = m_DesiredViewportLeft;
    m_CurrentState->m_ViewportWidth = m_DesiredViewportWidth;
    m_CurrentState->m_ViewportTop = m_DesiredViewportTop;
    m_CurrentState->m_ViewportHeight = m_DesiredViewportHeight;
    m_CurrentState->m_ViewportMinZ = m_DesiredViewportMinZ;
    m_CurrentState->m_ViewportMaxZ = m_DesiredViewportMaxZ;

    /*
    * Note: sadly RTXRemix honors SetViewport nor SetScissorRect's X/Y values.
    */

    D3DVIEWPORT9 d3dViewport;
    d3dViewport.X = *m_CurrentState->m_ViewportLeft;
    d3dViewport.Y = *m_CurrentState->m_ViewportTop;
    d3dViewport.Width = *m_CurrentState->m_ViewportWidth;
    d3dViewport.Height = *m_CurrentState->m_ViewportHeight;
    d3dViewport.MinZ = *m_CurrentState->m_ViewportMinZ;
    d3dViewport.MaxZ = *m_CurrentState->m_ViewportMaxZ;
    if (FAILED(m_Device->SetViewport(&d3dViewport)))
    {
      GLog->Logf(L"[EchelonRenderer]\t Failed to resized viewport to %dx%d", d3dViewport.Width, d3dViewport.Height);
      return false;
    }

    RECT r;
    r.left = d3dViewport.X;
    r.top = d3dViewport.Y;
    r.right = d3dViewport.X + d3dViewport.Width;
    r.bottom = d3dViewport.Y + d3dViewport.Height;
    m_Device->SetScissorRect(&r);
  }
  return true;
}

void LowlevelRenderer::ClearDepth()
{
  if (m_Device == nullptr) return;

  auto hr = m_Device->Clear(
    0,
    NULL,
    D3DCLEAR_ZBUFFER,
    0,
    1.0f,
    0);
  check(SUCCEEDED(hr));
}

void LowlevelRenderer::ClearDisplaySurface(const Vec4& clearColor)
{
  if (m_Device == nullptr) return;

  auto hr = m_Device->Clear(
    0,
    NULL,
    D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
    D3DCOLOR_XRGB(static_cast<uint8_t>(clearColor.x * 255.0f), static_cast<uint8_t>(clearColor.y * 255.0f), static_cast<uint8_t>(clearColor.z * 255.0f)),
    1.0f,
    0);
  check(SUCCEEDED(hr));
}

bool LowlevelRenderer::AllocateTexture(DeusExD3D9TextureHandle& pmTexture)
{
  auto result = m_Device->CreateTexture(
    pmTexture->md.width,
    pmTexture->md.height,
    1,
    0,
    pmTexture->format,
    D3DPOOL_MANAGED,
    &pmTexture->textureD3D9,
    NULL);
  if (FAILED(result))
  {
    return false;
  }

  D3DSURFACE_DESC desc{};
  pmTexture->textureD3D9->GetLevelDesc(0, &desc);

  D3DLOCKED_RECT data{};
  if (SUCCEEDED(pmTexture->textureD3D9->LockRect(0, &data, NULL, 0)))
  {
    memcpy(data.pBits, pmTexture->textureDataPtr, pmTexture->textureDataPitch * desc.Height);
    pmTexture->remixHash = XXH3_64bits(pmTexture->textureDataPtr, pmTexture->textureDataPitch * desc.Height);
    pmTexture->textureD3D9->UnlockRect(0);
    pmTexture->textureD3D9->AddRef();
    return true;
  }

  return false;
}

bool LowlevelRenderer::SetTextureOnDevice(const uint32_t pSlot, const DeusExD3D9Texture* pTexture)
{
  if (pTexture->valid)
  {
    assert(pSlot >=0 && pSlot < std::size(m_CurrentState->m_TextureSlots));
    if (!m_CurrentState->m_TextureSlots[pSlot] || *m_CurrentState->m_TextureSlots[pSlot] != pTexture->textureD3D9)
    {
      auto hr = m_Device->SetTexture(pSlot, pTexture->textureD3D9);
      if (SUCCEEDED(hr))
      {
        m_CurrentState->m_TextureSlots[pSlot] = pTexture->textureD3D9;
        return true;
      }
    }
    return true;
  }
  return false;
}

void LowlevelRenderer::ConfigureBlendState(UnrealBlendFlags pFlags)
{
  if ((pFlags & (PF_Translucent | PF_Modulated | PF_Highlighted | PF_Unlit)) != 0)
  {
    this->SetRenderState(D3DRS_ALPHABLENDENABLE, ((pFlags & PF_Masked) == 0) ? TRUE : FALSE);
    if ((pFlags & PF_Translucent) != 0) {
      if ((pFlags & PF_Portal) == 0)
      {
        this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
      }
      else
      {
        this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
      }
    }
    else if ((pFlags & PF_Modulated) != 0) {
      this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
      this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
    }
    else if ((pFlags & PF_Highlighted) != 0) {
      this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
    else if ((pFlags & PF_Unlit) != 0) {
      this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
    }
    else
    {
      //Are these defaults sane?
      this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR /*D3D9 default should be D3DBLEND_ONE*/);
      this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR /*D3D9 default should be D3DBLEND_ZERO*/);
    }
  }
  else
  {
    this->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR /*D3D9 default should be D3DBLEND_ONE*/);
    this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE /*D3D9 default should be D3DBLEND_ZERO*/);
  }




  if ((pFlags & PF_Masked) != 0)
  {
    this->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    this->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
  }
  else
  {
    this->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  }

  if ((pFlags & PF_Invisible) != 0)
  {
    this->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
  }
  else
  {
    this->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000f);
  }

  if ((pFlags & PF_Occlude) != 0)
  {
    this->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  }
  else
  {
    this->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
  }

  if ((pFlags & PF_RenderFog) != 0)
  {
    this->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
  }
  else
  {
    this->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
  }

  //This acts as an override to the above
  if (g_options.hasLights)
  {
    if ((pFlags & PF_Unlit) != 0)
    {
        this->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
        this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }
  }
  else
  {
    this->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD /*default*/);
  }


}

void LowlevelRenderer::ConfigureTextureStageState(int pStageID, UnrealBlendFlags pFlags)
{
  if ((pFlags & PF_Modulated) != 0)
  {
    this->SetTextureStageState(pStageID, D3DTSS_COLOROP, D3DTOP_MODULATE);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
  }
  else if ((pFlags & PF_Highlighted) != 0)
  {
    this->SetTextureStageState(pStageID, D3DTSS_COLOROP, D3DTOP_MODULATEINVALPHA_ADDCOLOR);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  }
  else
  {
    if (pStageID > 0)
    {
      this->SetTextureStageState( pStageID, D3DTSS_COLOROP, (pFlags&PF_Memorized) ? D3DTOP_MODULATE2X : D3DTOP_DISABLE );
      this->SetTextureStageState( pStageID, D3DTSS_ALPHAOP, (pFlags&PF_Memorized) ? D3DTOP_SELECTARG2 : D3DTOP_DISABLE );
      this->SetTextureStageState(pStageID, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_CURRENT);
    }
    else
    {
      this->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
      this->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      this->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      this->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }
  }
}

void LowlevelRenderer::ConfigureSamplerState(int pStageID, UnrealPolyFlags pFlags)
{
  if ((pFlags & PF_NoSmooth) != 0)
  {
    this->SetSamplerState(pStageID, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    this->SetSamplerState(pStageID, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    this->SetSamplerState(pStageID, D3DSAMP_MAXANISOTROPY, 1);
  }
  else
  {
    auto filter = D3DTEXF_ANISOTROPIC;
    auto anisotropyLevel = m_caps.MaxAnisotropy;
    g_DebugMenu.DebugVar("Rendering", "Tex Filter Type", DebugMenuUniqueID(), filter, { DebugMenuValueOptions::editor::slider, 0.0f,0.0f, 0, 8 });
    g_DebugMenu.DebugVar("Rendering", "Tex Filter Level", DebugMenuUniqueID(), anisotropyLevel, { DebugMenuValueOptions::editor::slider, 0.0f,0.0f, 0, m_caps.MaxAnisotropy });
    this->SetSamplerState(pStageID, D3DSAMP_MINFILTER, filter);
    this->SetSamplerState(pStageID, D3DSAMP_MAGFILTER, filter);
    this->SetSamplerState(pStageID, D3DSAMP_MAXANISOTROPY, anisotropyLevel);
  }
}

void LowlevelRenderer::InitializeDeviceState()
{
  constexpr D3DRENDERSTATETYPE states[] = {
    D3DRS_ZENABLE, 
    D3DRS_FILLMODE, 
    D3DRS_SHADEMODE, 
    D3DRS_ZWRITEENABLE, 
    D3DRS_ALPHATESTENABLE, 
    D3DRS_LASTPIXEL, 
    D3DRS_SRCBLEND, 
    D3DRS_DESTBLEND, 
    D3DRS_CULLMODE, 
    D3DRS_ZFUNC, 
    D3DRS_ALPHAREF, 
    D3DRS_ALPHAFUNC, 
    D3DRS_DITHERENABLE, 
    D3DRS_ALPHABLENDENABLE, 
    D3DRS_FOGENABLE, 
    D3DRS_SPECULARENABLE, 
    D3DRS_FOGCOLOR, 
    D3DRS_FOGTABLEMODE, 
    D3DRS_FOGSTART, 
    D3DRS_FOGEND, 
    D3DRS_FOGDENSITY, 
    D3DRS_RANGEFOGENABLE, 
    D3DRS_STENCILENABLE, 
    D3DRS_STENCILFAIL, 
    D3DRS_STENCILZFAIL, 
    D3DRS_STENCILPASS, 
    D3DRS_STENCILFUNC, 
    D3DRS_STENCILREF, 
    D3DRS_STENCILMASK, 
    D3DRS_STENCILWRITEMASK, 
    D3DRS_TEXTUREFACTOR, 
    D3DRS_WRAP0, 
    D3DRS_WRAP1, 
    D3DRS_WRAP2, 
    D3DRS_WRAP3, 
    D3DRS_WRAP4, 
    D3DRS_WRAP5, 
    D3DRS_WRAP6, 
    D3DRS_WRAP7, 
    D3DRS_CLIPPING, 
    D3DRS_LIGHTING, 
    D3DRS_AMBIENT, 
    D3DRS_FOGVERTEXMODE, 
    D3DRS_COLORVERTEX, 
    D3DRS_LOCALVIEWER, 
    D3DRS_NORMALIZENORMALS, 
    D3DRS_DIFFUSEMATERIALSOURCE, 
    D3DRS_SPECULARMATERIALSOURCE, 
    D3DRS_AMBIENTMATERIALSOURCE, 
    D3DRS_EMISSIVEMATERIALSOURCE, 
    D3DRS_VERTEXBLEND, 
    D3DRS_CLIPPLANEENABLE, 
    D3DRS_POINTSIZE, 
    D3DRS_POINTSIZE_MIN, 
    D3DRS_POINTSPRITEENABLE, 
    D3DRS_POINTSCALEENABLE, 
    D3DRS_POINTSCALE_A, 
    D3DRS_POINTSCALE_B, 
    D3DRS_POINTSCALE_C, 
    D3DRS_MULTISAMPLEANTIALIAS, 
    D3DRS_MULTISAMPLEMASK, 
    D3DRS_PATCHEDGESTYLE, 
    D3DRS_DEBUGMONITORTOKEN, 
    D3DRS_POINTSIZE_MAX, 
    D3DRS_INDEXEDVERTEXBLENDENABLE, 
    D3DRS_COLORWRITEENABLE, 
    D3DRS_TWEENFACTOR, 
    D3DRS_BLENDOP, 
    D3DRS_POSITIONDEGREE, 
    D3DRS_NORMALDEGREE, 
    D3DRS_SCISSORTESTENABLE, 
    D3DRS_SLOPESCALEDEPTHBIAS, 
    D3DRS_ANTIALIASEDLINEENABLE, 
    D3DRS_MINTESSELLATIONLEVEL, 
    D3DRS_MAXTESSELLATIONLEVEL, 
    D3DRS_ADAPTIVETESS_X, 
    D3DRS_ADAPTIVETESS_Y, 
    D3DRS_ADAPTIVETESS_Z, 
    D3DRS_ADAPTIVETESS_W, 
    D3DRS_ENABLEADAPTIVETESSELLATION, 
    D3DRS_TWOSIDEDSTENCILMODE, 
    D3DRS_CCW_STENCILFAIL, 
    D3DRS_CCW_STENCILZFAIL, 
    D3DRS_CCW_STENCILPASS, 
    D3DRS_CCW_STENCILFUNC, 
    D3DRS_COLORWRITEENABLE1, 
    D3DRS_COLORWRITEENABLE2, 
    D3DRS_COLORWRITEENABLE3, 
    D3DRS_BLENDFACTOR, 
    D3DRS_SRGBWRITEENABLE, 
    D3DRS_DEPTHBIAS, 
    D3DRS_WRAP8, 
    D3DRS_WRAP9, 
    D3DRS_WRAP10, 
    D3DRS_WRAP11, 
    D3DRS_WRAP12, 
    D3DRS_WRAP13, 
    D3DRS_WRAP14, 
    D3DRS_WRAP15, 
    D3DRS_SEPARATEALPHABLENDENABLE, 
    D3DRS_SRCBLENDALPHA, 
    D3DRS_DESTBLENDALPHA, 
    D3DRS_BLENDOPALPHA
  };

  for (const auto s : states)
  {
    DWORD d=0;
    if (m_Device->GetRenderState(s, &d) == D3D_OK)
    {
      m_CurrentState->m_RenderStates[s] = d;
    }
  }
}

void LowlevelRenderer::PushDeviceState()
{
  const bool canPush = ((m_CurrentState + 1) < &m_States[std::size(m_States) - 1]);
  check(canPush); //fails if there's an unmatched pop for a push.
  if (canPush)
  {
    m_CurrentState++;
    *m_CurrentState = *(m_CurrentState - 1);
  }
}

void LowlevelRenderer::PopDeviceState()
{
  const auto device = m_Device;
  const bool canPop = ((m_CurrentState - 1) >= &m_States[0]);
  check(canPop);
  if (canPop)
  {
    auto pendingState = (m_CurrentState - 1);
    //Restore states:
    for (int i = 0; i < std::size(m_CurrentState->m_RenderStates); i++)
    {
      auto& currentRs = m_CurrentState->m_RenderStates[i];
      auto& pendingRs = pendingState->m_RenderStates[i];
      check(
        (!currentRs.has_value() && !pendingRs.has_value()) ||
        (currentRs.has_value() && pendingRs.has_value())
      );

      if (pendingRs)
      {
        m_Device->SetRenderState(D3DRENDERSTATETYPE(i), *pendingRs);
      }
    }

    for (int i = 0; i < std::size(pendingState->m_TextureSlots); i++)
    {

      auto& pendingTexSlot = (pendingState->m_TextureSlots[i]);
      auto& currentTexSlot = (m_CurrentState->m_TextureSlots[i]);
      if (pendingTexSlot != currentTexSlot)
      {
        //TODO: when releasing a texture, crawl up through all states and remove it from any slots.
        //m_Device->SetTexture(i, pendingTexSlot ? *pendingTexSlot : nullptr);
      }
    }

    for (int stageId = 0; stageId < pendingState->MAX_TEXTURESTAGES; stageId++)
    {
      for (int stateId = 0; stateId < pendingState->MAX_TEXTURESTAGESTATES; stateId++)
      {
        auto& slot = pendingState->m_TextureStageStates[stageId][stateId];
        if (slot)
        {
          m_Device->SetTextureStageState(stageId, D3DTEXTURESTAGESTATETYPE(stateId), *slot);
        }
      }
      for (int samplerStateId = 0; samplerStateId < pendingState->MAX_SAMPLERSTATES; samplerStateId++)
      {
        auto& slot = pendingState->m_SamplerStates[stageId][samplerStateId];
        if (slot)
        {
          m_Device->SetSamplerState(stageId, D3DSAMPLERSTATETYPE(samplerStateId), *slot);
        }
      }
    }

    auto checkMatrix = [device](D3DTRANSFORMSTATETYPE pState, std::optional<D3DMATRIX>& pMatrixSlotNew, std::optional<D3DMATRIX>& pMatrixSlotOld) 
    {
      if (pMatrixSlotNew && pMatrixSlotOld)
      {
        auto& newM = *(pMatrixSlotNew);
        auto& oldM = *(pMatrixSlotOld);
        if (memcmp(&newM, &oldM, sizeof(D3DXMATRIX)) != 0)
        {
          device->SetTransform(pState, &newM);
        }
      }
      else
      {
        int x = 1;
      }
    };

    checkMatrix(D3DTS_WORLD, pendingState->m_WorldMatrix, m_CurrentState->m_WorldMatrix);
    checkMatrix(D3DTS_VIEW, pendingState->m_ViewMatrix, m_CurrentState->m_ViewMatrix);
    checkMatrix(D3DTS_PROJECTION, pendingState->m_ProjectionMatrix, m_CurrentState->m_ProjectionMatrix);

    //this->SetWorldMatrix(*(newState->m_WorldMatrix));
    //this->SetViewMatrix(*(newState->m_ViewMatrix));
    //this->SetProjectionMatrix(*(newState->m_ProjectionMatrix));
    SetViewport(*pendingState->m_ViewportLeft, *pendingState->m_ViewportTop, *pendingState->m_ViewportWidth, *pendingState->m_ViewportHeight);
    m_CurrentState--;
  }
}

DWORD LowlevelRenderer::GetRenderState(D3DRENDERSTATETYPE State) const
{
  check(State >= 0 && State < m_CurrentState->MAX_RENDERSTATES);

  if (m_CurrentState->m_RenderStates[State])
  {
    return *m_CurrentState->m_RenderStates[State];
  }
  else
  {
    DWORD v = 0;
    m_Device->GetRenderState(State, &v);
    return v;
  }
}

DWORD LowlevelRenderer::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE State) const
{
  check(State < m_CurrentState->MAX_TEXTURESTAGESTATES);

  if (m_CurrentState->m_TextureStageStates[Stage][State])
  {
    return *m_CurrentState->m_TextureStageStates[Stage][State];
  }
  else
  {
    DWORD v = 0;
    m_Device->GetTextureStageState(Stage, State, &v);
    return v;
  }
}

HRESULT LowlevelRenderer::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
  check(State < m_CurrentState->MAX_RENDERSTATES);
  if (!m_CurrentState->m_RenderStates[State] || *m_CurrentState->m_RenderStates[State] != Value)
  {
    m_CurrentState->m_RenderStates[State] = Value;
    return m_Device->SetRenderState(State, Value);
  }
  return S_OK;
}

HRESULT LowlevelRenderer::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
  check(Stage < m_CurrentState->MAX_TEXTURESTAGES);
  check(Type < m_CurrentState->MAX_TEXTURESTAGESTATES);
  if (!m_CurrentState->m_TextureStageStates[Stage][Type] || *m_CurrentState->m_TextureStageStates[Stage][Type] != Value)
  {
    m_CurrentState->m_TextureStageStates[Stage][Type] = Value;
    return m_Device->SetTextureStageState(Stage, Type, Value);
  }
  return S_OK;
}

HRESULT LowlevelRenderer::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
  check(Sampler < m_CurrentState->MAX_TEXTURESTAGES);
  check(Type < m_CurrentState->MAX_SAMPLERSTATES);
  if (!m_CurrentState->m_SamplerStates[Sampler][Type] || *m_CurrentState->m_SamplerStates[Sampler][Type] != Value)
  {
    m_CurrentState->m_SamplerStates[Sampler][Type] = Value;
    return m_Device->SetSamplerState(Sampler, Type, Value);
  }
  return S_OK;
}

std::vector<D3DDISPLAYMODE> LowlevelRenderer::GetDisplayModes() const
{
  std::vector<D3DDISPLAYMODE> displayModes;
  static std::vector<D3DDISPLAYMODE> lastDisplayModes;

  if (!m_API)
  {
    return lastDisplayModes;
  }

  const auto modeCount = m_API->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
  if (modeCount == 0)
  {
    //HACK: might be a remix bug, but, there were some cases where this briefly returned 0 for all adapters, when resizing.
    return lastDisplayModes;
  }

  for (auto modeIndex = 0; modeIndex < modeCount; modeIndex++) {
    D3DDISPLAYMODE modeInfo{};
    if (SUCCEEDED(m_API->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, modeIndex, &modeInfo))) {
      displayModes.push_back(modeInfo);
    }
  }

  std::sort(displayModes.begin(), displayModes.end(), [](const D3DDISPLAYMODE& pLH, const D3DDISPLAYMODE& pRH) {
    return ((pLH.Width > pRH.Width) || ((pLH.Width == pRH.Width && pLH.Height > pRH.Height)));
    });

  if (displayModes.size() > lastDisplayModes.size())
  {
    lastDisplayModes = displayModes;
  }

  return displayModes;
}

std::optional<D3DDISPLAYMODE> LowlevelRenderer::FindClosestResolution(uint32_t pWidth, uint32_t pHeight) const
{
  auto displayModes = GetDisplayModes();
  const int64_t surfaceTarget = pWidth * pHeight;
  std::sort(displayModes.begin(), displayModes.end(), [&](const D3DDISPLAYMODE& pLH, const D3DDISPLAYMODE& pRH) {
    int64_t surfaceDeltaLH = std::abs(surfaceTarget - int64_t(pLH.Width * pLH.Height));
    int64_t surfaceDeltaRH = std::abs(surfaceTarget - int64_t(pRH.Width * pRH.Height));
    int64_t widthDeltaLH = std::abs(int64_t(pWidth - pLH.Width));
    int64_t widthDeltaRH = std::abs(int64_t(pWidth - pRH.Width));
    return (surfaceDeltaLH < surfaceDeltaRH) && (widthDeltaLH < widthDeltaRH);
    });

  if (!displayModes.empty())
  {
    return displayModes.front();
  }

  return {};
}

const RangeDefinition& RenderRanges::FromContext(FrameContextManager::Context* pCtx)
{
  //We play with the ranges to help tweak RTX Remix, but when we're running fully
  //rasterized, we shouldn't care about that.
  if (pCtx->frameIsRasterized)
  {
    return Game;
  }

  if (pCtx->renderingUI)
  {
    return UI;
  }

  if (pCtx->frameIsSkybox)
  {
    return Skybox;
  }

  return Game;
}
