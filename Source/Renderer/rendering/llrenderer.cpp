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
#include "utils/debugmenu.h"

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

  D3DPRESENT_PARAMETERS d3dpp{0};
  d3dpp.Windowed = !pFullscreen;
  d3dpp.hDeviceWindow = hWnd;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = (pFullscreen ? D3DFMT_X8R8G8B8 : D3DFMT_UNKNOWN);
  d3dpp.BackBufferWidth = m_outputSurface.width;
  d3dpp.BackBufferHeight = m_outputSurface.height;
  d3dpp.BackBufferCount = 1;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
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
    D3DFMT_D16, // Depth buffer format (should match the one specified in d3dpp)
    D3DMULTISAMPLE_NONE, // Multisample type
    0, // Multisample quality
    TRUE, // Discard stencil data (if any)
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

  ///

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
  HRESULT res{};
  res = m_Device->BeginScene();
  check(SUCCEEDED(res));
}

void LowlevelRenderer::EndScene()
{
  auto res = m_Device->EndScene();
  check(SUCCEEDED(res));
}

void LowlevelRenderer::RenderTriangleListBuffer(DWORD pFVF, const void* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pVertexSize, const uint32_t pHash, const uint32_t pDebug)
{
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

  {
    HRESULT res = S_OK;
    res = m_Device->SetStreamSource(0, buffer, 0, pVertexSize); check(SUCCEEDED(res));
    res = m_Device->SetIndices(nullptr); check(SUCCEEDED(res));
    m_Device->SetVertexShaderConstantI(pDebug, nullptr, 0);
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

void LowlevelRenderer::DisableLight(int32_t index)
{
  if (!g_options.hasLights)
  {
    return;
  }

  m_Device->LightEnable(index, FALSE);
}

void LowlevelRenderer::RenderLight(int32_t index, const D3DLIGHT9& pLight)
{
  if (!g_options.hasLights)
  {
    return;
  }

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
    res = this->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    res = this->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    res = this->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    res = this->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    res = this->SetRenderState(D3DRS_ALPHAREF, 127);
    res = this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    res = this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    res = this->SetRenderState(D3DRS_LIGHTING, g_options.hasLights ? TRUE : FALSE); check(SUCCEEDED(res));
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

    if (!m_CurrentState->m_WorldMatrix)
    {
      m_CurrentState->m_WorldMatrix = D3DXMATRIX();
    }
    m_Device->GetTransform(D3DTS_WORLD, &m_CurrentState->m_WorldMatrix.value());

    ////Set view matrix
    //D3DMATRIX d3dView = { +1.0f,  0.0f,  0.0f,  0.0f,
    //	0.0f, -1.0f,  0.0f,  0.0f,
    //	0.0f,  0.0f, -1.0f,  0.0f,
    //	0.0f,  0.0f,  0.0f, +1.0f };
    //res = m_Device->SetTransform(D3DTS_VIEW, &d3dView);  check(SUCCEEDED(res));
  }

  //Start rendering
  auto res = m_Device->BeginScene();	check(SUCCEEDED(res));
  ClearDisplaySurface(Vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
}

void LowlevelRenderer::EndFrame()
{
  if (!m_IsInFrame)
  {
    return;
  }

  static auto lastTime = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  int timeSinceLastTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>((now - lastTime)).count();
  lastTime = now;
  if (m_Device == nullptr) return;
  auto res = m_Device->EndScene(); check(SUCCEEDED(res));

#if 1
  res = m_Device->Present(NULL, NULL, NULL, NULL);
#else
  static std::mutex presentMutex;
  static std::condition_variable canPresentCV;
  static bool canPresent = false;
  static bool hasPresented = true;
  static std::thread presentThread([&]() {
    while (true)
    {
      std::unique_lock lk(presentMutex);
      canPresentCV.wait(lk, [] { return canPresent; });

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
    canPresentCV.wait(lk, [] { return hasPresented; });
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

void LowlevelRenderer::SetWorldMatrix(const D3DMATRIX& pMatrix)
{
  if (!m_CurrentState->m_WorldMatrix || ::memcmp(&(m_CurrentState->m_WorldMatrix.value()), &pMatrix, sizeof(pMatrix)) != 0)
  {
    auto result = m_Device->SetTransform(D3DTS_WORLD, &pMatrix);
    m_CurrentState->m_WorldMatrix = pMatrix;
    check(SUCCEEDED(result));
  }
}

void LowlevelRenderer::SetViewMatrix(const D3DMATRIX& pMatrix)
{
  if (!m_CurrentState->m_ViewMatrix || ::memcmp(&(m_CurrentState->m_ViewMatrix.value()), &pMatrix, sizeof(pMatrix)) != 0)
  {
    auto result = m_Device->SetTransform(D3DTS_VIEW, &pMatrix);
    m_CurrentState->m_ViewMatrix = pMatrix;
    check(SUCCEEDED(result));
  }
}

void LowlevelRenderer::SetProjectionMatrix(const D3DMATRIX& pMatrix)
{
  if (!m_CurrentState->m_ProjectionMatrix || ::memcmp(&(m_CurrentState->m_ProjectionMatrix.value()), &pMatrix, sizeof(pMatrix)) != 0)
  {
    auto result = m_Device->SetTransform(D3DTS_PROJECTION, &pMatrix);
    m_CurrentState->m_ProjectionMatrix = pMatrix;
    check(SUCCEEDED(result));
  }
}

bool LowlevelRenderer::SetViewport(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight)
{
  if (g_options.enableViewportXYOffsetWorkaround)
  {
    m_DesiredViewportLeft = 0;
    m_DesiredViewportTop = 0;
    m_DesiredViewportWidth = m_outputSurface.width;
    m_DesiredViewportHeight = m_outputSurface.height;
  }
  else
  {
    m_DesiredViewportLeft = pLeft;
    m_DesiredViewportTop = pTop;
    m_DesiredViewportWidth = pWidth;
    m_DesiredViewportHeight = pHeight;
  }

  if (!m_DesiredViewportMinZ)
  {
    m_DesiredViewportMinZ = LowlevelRenderer::NearRange;
  }

  if (!m_DesiredViewportMaxZ)
  {
    m_DesiredViewportMaxZ = LowlevelRenderer::FarRange;
  }
  return ValidateViewport();
}

void LowlevelRenderer::SetViewportDepth(float pMinZ, float pMaxZ)
{
  m_DesiredViewportMinZ = pMinZ;
  m_DesiredViewportMaxZ = pMaxZ;
  ValidateViewport();
}

void LowlevelRenderer::ResetViewportDepth()
{
  m_DesiredViewportMinZ = LowlevelRenderer::NearRange;
  m_DesiredViewportMaxZ = LowlevelRenderer::FarRange;
  ValidateViewport();
}

bool LowlevelRenderer::ValidateViewport()
{
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

bool LowlevelRenderer::SetTextureOnDevice(const DeusExD3D9Texture* pTexture)
{
  constexpr uint32_t slot = 0;

  if (pTexture->valid)
  {
    if (!m_CurrentState->m_TextureSlots[slot] || *m_CurrentState->m_TextureSlots[slot] != pTexture->textureD3D9)
    {
      auto hr = m_Device->SetTexture(slot, pTexture->textureD3D9);
      if (SUCCEEDED(hr))
      {
        m_CurrentState->m_TextureSlots[slot] = pTexture->textureD3D9;
        return true;
      }
    }
    return true;
  }
  return false;
}

void LowlevelRenderer::ConfigureBlendState(UnrealBlendFlags pFlags)
{
  auto handleflag = [&](UnrealBlendFlags pFlag, std::function<void()> onSet, std::function<void()> onUnset)
    {
      if ((pFlags & pFlag) != 0)
      {
        onSet();
      }
      else
      {
        onUnset();
      }
    };

  //Masked is disabled because the renderer (currently) does not support masking.
  handleflag(PF_Translucent | PF_Modulated | PF_Highlighted | PF_Unlit,
    [&]() {
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
        this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
      }
    },
    [&]()
    {
      this->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
  );




  handleflag(PF_Masked,
    [&]() {
      this->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
      this->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
      this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    },
    [&]()
    {
      this->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }
  );

  handleflag(PF_Invisible,
    [&]() {
      this->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
    },
    [&]()
    {
      this->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000f);
    }
  );

  handleflag(PF_Occlude,
    [&]() {
      this->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    },
    [&]()
    {
      this->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
  );

  handleflag(PF_RenderFog,
    [&]() {
      this->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    },
    [&]()
    {
      this->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    }
  );

  //This acts as an override to the above
  if (g_options.hasLights)
  {
    handleflag(PF_Unlit,
      [&]() {
        this->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        this->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
        this->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
      },
      [&]()
      {
        //this->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
      }
    );
  }


}

void LowlevelRenderer::ConfigureTextureStageState(int pStageID, UnrealBlendFlags pFlags)
{
  if (pStageID == 0)
  {
    this->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    this->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  }

  if ((pFlags & PF_Modulated) != 0)
  {
    this->SetTextureStageState(pStageID, D3DTSS_COLOROP, D3DTOP_MODULATE);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_CURRENT);
  }
  else if ((pFlags & PF_Memorized) != 0)
  {
    this->SetTextureStageState(pStageID, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  }
  else if ((pFlags & PF_Highlighted) != 0)
  {
    this->SetTextureStageState(pStageID, D3DTSS_COLOROP, D3DTOP_MODULATEINVALPHA_ADDCOLOR);
    this->SetTextureStageState(pStageID, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    this->SetTextureStageState(pStageID, D3DTSS_COLORARG2, D3DTA_CURRENT);
  }
}

void LowlevelRenderer::ConfigureSamplerState(int pStageID, UnrealPolyFlags pFlags)
{
  if ((pFlags & PF_NoSmooth) == 0)
  {
    auto filter = D3DTEXF_ANISOTROPIC;
    auto anisotropyLevel = m_caps.MaxAnisotropy;
    g_DebugMenu.DebugVar("Rendering", "Tex Filter Type", DebugMenuUniqueID(), filter, { DebugMenuValueOptions::editor::slider, 0.0f,0.0f, 0, 8 });
    g_DebugMenu.DebugVar("Rendering", "Tex Filter Level", DebugMenuUniqueID(), anisotropyLevel, { DebugMenuValueOptions::editor::slider, 0.0f,0.0f, 0, m_caps.MaxAnisotropy });
    this->SetSamplerState(pStageID, D3DSAMP_MINFILTER, filter);
    this->SetSamplerState(pStageID, D3DSAMP_MAGFILTER, filter);
    this->SetSamplerState(pStageID, D3DSAMP_MAXANISOTROPY, anisotropyLevel);
  }
  else
  {
    this->SetSamplerState(pStageID, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    this->SetSamplerState(pStageID, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    this->SetSamplerState(pStageID, D3DSAMP_MAXANISOTROPY, 1);
  }
}

void LowlevelRenderer::PushDeviceState()
{
  const bool canPush = ((m_CurrentState + 1) < &m_States[std::size(m_States) - 1]);
  check(canPush);
  if (canPush)
  {
    m_CurrentState++;
    *m_CurrentState = *(m_CurrentState - 1);
  }
}

void LowlevelRenderer::PopDeviceState()
{
  const bool canPop = ((m_CurrentState - 1) >= &m_States[0]);
  check(canPop);
  if (canPop)
  {
    auto newState = (m_CurrentState - 1);
    //Restore states:
    for (int i = 0; i < std::size(newState->m_RenderStates); i++)
    {
      auto& rs = newState->m_RenderStates[i];
      if (rs)
      {
        m_Device->SetRenderState(D3DRENDERSTATETYPE(i), *rs);
      }
    }

    for (int i = 0; i < std::size(newState->m_TextureSlots); i++)
    {

      auto& texSlot = (newState->m_TextureSlots[i]);
      m_Device->SetTexture(i, texSlot ? *texSlot : nullptr);
    }

    for (int stageId = 0; stageId < newState->MAX_TEXTURESTAGES; stageId++)
    {
      for (int stateId = 0; stateId < newState->MAX_TEXTURESTAGESTATES; stateId++)
      {
        auto& slot = newState->m_TextureStageStates[stageId][stateId];
        if (slot)
        {
          m_Device->SetTextureStageState(stageId, D3DTEXTURESTAGESTATETYPE(stateId), *slot);
        }
      }
      for (int samplerStateId = 0; samplerStateId < newState->MAX_SAMPLERSTATES; samplerStateId++)
      {
        auto& slot = newState->m_SamplerStates[stageId][samplerStateId];
        if (slot)
        {
          m_Device->SetSamplerState(stageId, D3DSAMPLERSTATETYPE(samplerStateId), *slot);
        }
      }
    }
    check(newState->m_WorldMatrix);
    check(newState->m_ViewMatrix);
    check(newState->m_ProjectionMatrix);

    this->SetWorldMatrix(*(newState->m_WorldMatrix));
    this->SetViewMatrix(*(newState->m_ViewMatrix));
    this->SetProjectionMatrix(*(newState->m_ProjectionMatrix));
    SetViewport(*newState->m_ViewportLeft, *newState->m_ViewportTop, *newState->m_ViewportWidth, *newState->m_ViewportHeight);
    m_CurrentState--;
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