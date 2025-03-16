#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop
#include "uefacade.h"
#include <MurmurHash3.cpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <FCNTL.H>
#include <vector>
#include <intrin.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <chrono>

#include "MurmurHash3.h"
#include <Core/hooks/hooks.h>
#include <Core/demomanager.h>
#include <Core/commandmanager.h>
#include "utils/configmanager.h"
#include "utils/debugmenu.h"
#include "utils/utils.h"
#include "hacks/hacks.h"
#include "hacks/misc.h"
#include "rendering/scenemanager.h"

IMPLEMENT_PACKAGE(DeusExEchelonRenderer);
IMPLEMENT_CLASS(UD3D9FPRenderDevice);
IMPLEMENT_CLASS(UBenchmark);

LowlevelRenderer UD3D9FPRenderDevice::m_LLRenderer;
HighlevelRenderer UD3D9FPRenderDevice::m_HLRenderer;
std::mutex UD3D9FPRenderDevice::m_Lock;

class FLogOutImpl : public FOutputDevice
{
private:
  FOutputDevice* Parent = nullptr;
public:
  FLogOutImpl(FOutputDevice* pParent) : Parent(pParent) {};
  void Serialize(const TCHAR* V, EName Event)
  {
    //char b[1024]{};
    //WideCharToMultiByte(CP_ACP, 0, V, -1, &b[0], std::size(b), NULL, NULL);
    //printf("[%d] %s\n", int(Event), b);
    //fflush(stdout);
    OutputDebugStringW(V);
    OutputDebugStringW(L"\n");

    Parent->Serialize(V, Event);
  }
} LogOutImpl(GLog);

class FErrOutImpl : public FOutputDeviceError
{
public:
  void Serialize(const TCHAR* V, EName Event)
  {
    char b[1024]{};
    WideCharToMultiByte(CP_ACP, 0, V, -1, &b[0], std::size(b), NULL, NULL);
    printf("[NORMALLY-FATAL][%d] %s\n", int(Event), b);
    fflush(stdout);
  }
  void HandleError() {};
} ErrOutImpl;

void UD3D9FPRenderDevice::StaticConstructor()
{
}

UD3D9FPRenderDevice::UD3D9FPRenderDevice()
{
  //I don't know why, but, we _need_ to declare a constructor here.
  //default constructor somehow leads to problems.
}

UBOOL UD3D9FPRenderDevice::Init(UViewport* pInViewport, int32_t pWidth, int32_t pHeight, int32_t pColorBytes, UBOOL pFullscreen)
{
  std::unique_lock lock(m_Lock);
  GLog->Logf(L"[EchelonRenderer]\t UD3D9FPRenderDevice init called with: width:%d height:%d fullscreen:%d\n", pWidth, pHeight, pFullscreen!=0);
  if (GetClass() == nullptr)
  {
    return FALSE;
  }
  GRenderDevice = this;
  const HWND hwnd = (HWND)pInViewport->GetWindow();

  // Get device context and draw text
  auto hFont = (HFONT)GetStockObject(SYSTEM_FONT);
  HDC hdc = GetDC(hwnd);
  if (hFont = (HFONT)SelectObject(hdc, hFont))
  {
    std::wstring txt = L"Initializing Echelon Renderer.";
    TextOut(hdc, 50, 50, txt.c_str(), txt.length());
    SIZE txtSize;
    GetTextExtentPoint32(hdc, txt.c_str(), txt.length(), &txtSize);

    txt = L"May take a while on first-boot, new RTX Remix version or new GPU driver version...";
    TextOut(hdc, 50, 50 + (float(txtSize.cy) * 1.5f), txt.c_str(), txt.length());
    
  }
  ReleaseDC(hwnd, hdc);

  /*
    Deus Ex internally relies on a statically allocated array (in the .data) section
    that has a maximum of 2880 rows. This is used by the internal software rasterizer
    used for culling.
    I only see three locations where this is accessed:
    - URender::OccludeBsp(FSceneNode *)+F35         8D 14 C5 08 0C B5 10                    lea     edx, unk_10B50C08[eax*8]
    - URender::OccludeBsp(FSceneNode *)+F57         8D 0C C5 08 0C B5 10                    lea     ecx, unk_10B50C08[eax*8]
    - SetupRaster +19EF0                            8D 0C 85 08 0C B5 10                    lea     ecx, ds:10B50C08h[eax*4]
    We could use dynamically allocate a bigger buffer and patch the code (maybe using Zydis).
  */

  if (pHeight > 2880)
  {
    pHeight = min(pHeight, 2880);
    MessageBox(hwnd, L"Deus Ex does not support render resolutions beyond 2880 pixels tall.\nThe resolution has been capped.", L"Error, maximum resolution exceeded", MB_OK|MB_ICONERROR);
  }

#if !defined(RELEASE_CONFIG)
  //AllocConsole();
  //freopen("CONOUT$", "w", stdout);
  GNull = &LogOutImpl;
  GLog = &LogOutImpl;
  GError = &ErrOutImpl;
  //if( GLogWindow )
  //{
  //  GLogWindow->Show(1);
  //  SetFocus( *GLogWindow );
  //  GLogWindow->Display.ScrollCaret();
  //}
  pInViewport->Exec(L"showlog");
  //GNull = GLog;
  GError = &ErrOutImpl;
#endif

  InitializeEchelonCore();

  g_ConfigManager.LoadConfig();
  //g_ConfigManager.SaveConfig(); //save file again so that we have the defaults
  
  g_CommandManager.Initialize();
  g_DemoManager.Initialize();

  InstallRTXConfigPatches();
  InstallBytePatches();
  InstallThreadAffinityHacks();
  InstallFSceneNodeHacks();
  InstallFSpanBufferHacks();
  InstallURenderHacks();
  InstallUMeshHacks();
  InstallUGameEngineHooks();
  InstallFFDynamicSpriteHacks();
  InstallFDynamicItemFilterHacks();
  InstallULightManagerHacks();
  InstallUConsoleHacks();
  InstallXViewportWindowHacks();
  InstallAPlayerPawnHacks();

  Hooks::UGameEngineCallbacks::OnTick.insert({this, [&](FLOAT deltaTime){ Tick(deltaTime);}});
  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.insert({this, [&](){ OnLevelChange();}});

  Misc::setRendererFacade(this);
  GLog->Log(L"[EchelonRenderer]\t Initializing Direct3D 9 Fixed-Function Pipeline renderer.");
  g_DebugMenu.Init();

  //We set the following variables to inform the engine of our capabilities. 
  URenderDevice::SpanBased = 0;
  URenderDevice::FullscreenOnly = 0;
  URenderDevice::SupportsFogMaps = 1;
  URenderDevice::SupportsTC = 1;
  URenderDevice::SupportsDistanceFog = 1;
  URenderDevice::SupportsLazyTextures = 1;
  URenderDevice::Coronas = 1;
  URenderDevice::DetailTextures = 0;
  URenderDevice::ShinySurfaces = 1;
  URenderDevice::HighDetailActors = 1;
  URenderDevice::VolumetricLighting = 1;
  URenderDevice::PrefersDeferredLoad = 0;
  URenderDevice::PrecacheOnFlip = 1;

  auto c = this->GetClass();
  auto n = this->GetClass()->GetName();
  const auto blitType = (pFullscreen ? (BLIT_Fullscreen | BLIT_Direct3D) : (BLIT_HardwarePaint | BLIT_Direct3D));
  URenderDevice::Viewport = pInViewport;

  GWarn->Log(L"[EchelonRenderer]\t Calling Viewport->ResizeViewport with -1,-1");
  UD3D9FPRenderDevice::Viewport->ResizeViewport(BLIT_HardwarePaint | BLIT_Direct3D);
  GWarn->Logf(L"[EchelonRenderer]\t Calling Viewport->ResizeViewport again with width:%d height:%d\n", pWidth, pHeight);
  UBOOL Result = UD3D9FPRenderDevice::Viewport->ResizeViewport(blitType, pWidth, pHeight, pColorBytes);
  if (!Result) {
    GError->Logf(L"[EchelonRenderer]\t Unreal Engine failed to resize the viewport with parameters {type:%x, width:%d, height:%d, color:%x}", blitType, pWidth, pHeight, pColorBytes);
    return 0;
  }

  bool rendererInitialized = false;
  constexpr auto allowedTries = 5;
  for (int i = 0; i < allowedTries; i++)
  {
    if (i + 1 == allowedTries)
    {
      pFullscreen = false;
    }
    GWarn->Logf(L"[EchelonRenderer]\t Calling attempt #%02d\n", i+1);
    if (!m_LLRenderer.Initialize(hwnd, pWidth, pHeight, pColorBytes, pFullscreen))
    {
      GWarn->Logf(L"[EchelonRenderer]\t Renderer failed to initialize with parameters {type: %x, width:%d, height:%d, color:%x}", blitType, pWidth, pHeight, pColorBytes);
      ::Sleep(500);
      continue;
    }
    rendererInitialized = true;
    break;
  }

  if (!rendererInitialized)
  {
    this->Exec(L"SHOWLOG", *GLog);
    GError->Log(L"Failed to initialize the low-level renderer. Please check DeusEx.log in either the game's System Folder or in the 'Deus Ex' folder in My Documents.");
    return 0;
  }

  if (!this->SetRes(pWidth, pHeight, pColorBytes, pFullscreen))
  {
    GError->Log(L"SetRes failed during initialization.");
    return false;
  }

  m_HLRenderer.Initialize(&m_LLRenderer);
  g_SceneManager.Initialize(&m_LLRenderer, &m_HLRenderer);

  //force lights off if running with remix disabled
  g_options.hasLights = g_options.hasLights && Misc::IsNvRemixAttached(true);

  //Other renderers solve a compatibility issue by setting the processor affinity to a single-cpu
  //Presumably to solve an issue with __rdtsc drift. For us, this screws with NV Remix.
  //If this is an issue, we can set thread affinity for Deus Ex's main thread and the rendering thread, instead.
  //SetProcessAffinityMask(GetCurrentProcess(),0x1); 
  //Misc::applyThreadAffinityMasks();

  auto m = GWindowManager;

  //Force directinput on, otherwise we're not able to interact with the RTX menu.
  if (g_ConfigManager.GetAggressiveMouseFix())
  {
    pInViewport->SetMouseCapture(TRUE, TRUE, FALSE);
    GConfig->SetBool(L"WinDrv.WindowsClient", L"CaptureMouse", TRUE, L"DeusEx.ini");
  }

  GConfig->SetBool(L"WinDrv.WindowsClient", L"UseDirectInput", FALSE, L"DeusEx.ini");
  GConfig->Flush(FALSE);

  return 1;
}

void UD3D9FPRenderDevice::Exit()
{
  std::unique_lock lock(m_Lock);
  GLog->Log(L"[EchelonRenderer]\t Direct3D 9 Fixed-Function Pipeline renderer exiting.");
  g_CommandManager.Shutdown();
  g_DemoManager.Shutdown();
  g_DebugMenu.Shutdown();
  g_SceneManager.Shutdown();
  m_HLRenderer.Shutdown();
  m_LLRenderer.Shutdown();
#if !defined(RELEASE_CONFIG)
  FreeConsole();
#endif

  Hooks::UGameEngineCallbacks::OnTick.erase(this);
  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.erase(this);

  UninstallULightManagerHacks();
  UninstallFDynamicItemFilterHacks();
  UninstallFFDynamicSpriteHacks();
  UninstallURenderHacks();
  UninstallUMeshHacks();
  UninstallFSpanBufferHacks();
  UninstallFSceneNodeHacks();
  UninstallThreadAffinityHacks();
  UninstallUGameEngineHooks();
  UninstallBytePatches();
  UninstallUConsoleHacks();
  UninstallXViewportWindowHacks();
  UninstallAPlayerPawnHacks();

  ShutdownEchelonCore();
}

UBOOL UD3D9FPRenderDevice::SetRes(INT pWidth, INT pHeight, INT pColorBytes, UBOOL pFullscreen)
{
  auto mx = URenderDevice::Viewport->GetOuterUClient()->WindowedViewportX;
  auto my = URenderDevice::Viewport->GetOuterUClient()->WindowedViewportY;
  m_TargetResWidth = pWidth;
  m_TargetResHeight = pHeight;
  m_TargetResColorBytes = pColorBytes;
  m_TargetResFullscreen = pFullscreen;
  GLog->Logf(L"[EchelonRenderer]\t Set called with: width:%d height:%d fullscreen:%d\n", m_TargetResWidth, m_TargetResHeight, m_TargetResFullscreen?1:0);

  if (!m_CurrentResWidth.has_value() ||
    !m_CurrentResHeight.has_value() ||
    !m_CurrentResColorBytes.has_value() ||
    !m_CurrentResFullscreen.has_value())
  {
    return ValidateRes();
  }

  return TRUE;
}

UBOOL UD3D9FPRenderDevice::ValidateRes()
{
  const bool hasChanged = (m_TargetResWidth != m_CurrentResWidth) || (m_TargetResHeight != m_CurrentResHeight) || (m_TargetResFullscreen != m_CurrentResFullscreen);
  if (!hasChanged)
  {
    return TRUE;
  }

  GLog->Logf(L"[EchelonRenderer]\t ValidateRes called with: width:%d height:%d fullscreen:%d\n", m_TargetResWidth, m_TargetResHeight, m_TargetResFullscreen?1:0);

  if (m_TargetResWidth == 0 || m_TargetResHeight == 0)
  {
    m_TargetResFullscreen = false;
    m_TargetResWidth = m_CurrentResWidth ? *m_CurrentResWidth : 128;
    m_TargetResHeight = m_CurrentResHeight ? *m_CurrentResHeight : 128;
  }

  if (m_TargetResFullscreen)
  {
    if (auto res = m_LLRenderer.FindClosestResolution(m_TargetResWidth, m_TargetResHeight); res)
    {
      GLog->Logf(L"[EchelonRenderer]\t Fullscreen Setres was requested, and matched %dx%d to available resolution %dx%d.", m_TargetResWidth, m_TargetResHeight, res->Width, res->Height);
      m_TargetResWidth = res->Width;
      m_TargetResHeight = res->Height;
    }
    else
    {
      m_TargetResFullscreen = false;
      GLog->Logf(L"[EchelonRenderer]\t SetRes failed, could not find matching fullscreen resolution for %dx%d, possible device returned no modes. Swithing to windowed.", m_TargetResWidth, m_TargetResHeight);
    }
  }

  const auto blitType = (m_TargetResFullscreen ? (BLIT_Fullscreen | BLIT_Direct3D) : (BLIT_HardwarePaint | BLIT_Direct3D));
  UBOOL Result = URenderDevice::Viewport->ResizeViewport(blitType, m_TargetResWidth, m_TargetResHeight, m_TargetResColorBytes);
  if (!Result)
  {
    GError->Log(L"SetRes failed: Error resizing viewport.");
    return 0;
  }
  if (!m_LLRenderer.ResizeDisplaySurface(0, 0, m_TargetResWidth, m_TargetResHeight, m_TargetResFullscreen))
  {
    GError->Log(L"SetRes failed due to internal error in m_LLRenderer.Resize()");
    return 0;
  }
#if 0
  //We need to calculate our FoV. Default is 75.
  //customFOV = Misc::getFov(defaultFOV,Viewport->SizeX,Viewport->SizeY);
  //Does this need to be set on Viewport->Actor->FovAngle?
#endif
  Result = m_LLRenderer.ResizeDisplaySurface(0, 0, m_TargetResWidth, m_TargetResHeight, m_TargetResFullscreen);
  if (Result)
  {
    m_CurrentResWidth = m_TargetResWidth;
    m_CurrentResHeight = m_TargetResHeight;
    m_CurrentResFullscreen = m_TargetResFullscreen;
    m_CurrentResColorBytes = m_TargetResColorBytes;
  }

  if (URenderDevice::Viewport && g_ConfigManager.GetAggressiveMouseFix())
  {
    URenderDevice::Viewport->SetMouseCapture(TRUE, FALSE, TRUE);
  }

  return Result;
}

void UD3D9FPRenderDevice::Flush(UBOOL AllowPrecache)
{
  if (!g_options.renderingDisabled)
  {
    //If caching is allowed, tell the game to make caching calls (PrecacheTexture() function)
    URenderDevice::PrecacheOnFlip = 1;
  }
}

static auto renderStart = std::chrono::high_resolution_clock::now();
void UD3D9FPRenderDevice::Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* InHitData, INT* InHitSize)
{
  std::unique_lock lock(m_Lock);
  
  renderStart = std::chrono::high_resolution_clock::now();
  if (!g_options.renderingDisabled)
  {
    static uint32_t lastHash = 0;
    static uint32_t frameIdx = 0;
    char fileName[255]{ 0 };
    //
    if (m_NotifyLevelHasChanged)
    {
      GetLLRenderer()->OnLevelChange();
      GetHLRenderer()->OnLevelChange();
      m_NotifyLevelHasChanged = false;
    }

    if (InHitData == nullptr)
    {
      this->Viewport->HitTesting = true;
    }
    m_LLRenderer.BeginFrame();
  }

}

/**
Finish rendering.
/param Blit Whether the front and back buffers should be swapped.
*/
void UD3D9FPRenderDevice::Unlock(UBOOL Blit)
{
  {
    static uint64_t lastFrameDurationIndex = 0;
    static uint64_t frameDurations[32]{};
    auto now = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - renderStart).count();
    frameDurations[lastFrameDurationIndex++] = frameDuration;
    lastFrameDurationIndex %= std::size(frameDurations);
    float avgFrameDurationMs = 0.0f;
    for (auto duration : frameDurations)
    {
      avgFrameDurationMs += float(duration);
    }
    avgFrameDurationMs /= std::size(frameDurations);
    static auto lastUpdate = now;
    if (now > lastUpdate + std::chrono::milliseconds(1000 * 10))
    {
      //GLog->Logf(L"[EchelonRenderer]\t Frame duration: %2f ms", avgFrameDurationMs);
      lastUpdate = now;
    }
  }

  if (!g_options.renderingDisabled)
  {
    if (Blit)
    {
      m_LLRenderer.EndFrame();
    }
  }

  ValidateRes();
}

void UD3D9FPRenderDevice::Tick(FLOAT DeltaTime)
{
  g_DebugMenu.Render();
  g_Stats.OnTick();
}

void UD3D9FPRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span)
{
  g_SceneManager.Validate();
  m_HLRenderer.OnDrawMeshPolygon(Frame, Info, Pts, NumPts, PolyFlags, Span);
}

void UD3D9FPRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
  g_SceneManager.Validate();
  m_HLRenderer.OnDrawGeometry(Frame, Surface, Facet);
}

void UD3D9FPRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, class FSpanBuffer* Span, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags)
{

  g_SceneManager.Validate();
  std::wstring textureName = Info.Texture->GetFullName();
  m_LLRenderer.EmitDebugTextF(L"[EchelonRenderer] DrawTile: %s", textureName.c_str());
  if (textureName.find(L"Corona") != -1)
  {
    constexpr float scale = 0.5f;
    constexpr float half = 0.5f;
    const float dx = XL - (XL * scale);
    const float dy = YL - (YL * scale);
    X += (dx * half);
    Y += (dy * half);
    XL *= scale;
    YL *= scale;
  }

  if (Z > 1.0f)
  {
    m_HLRenderer.OnDrawSprite(Frame, Info, X, Y, XL, YL, U, V, UL, VL, Span, Z, Color, Fog, PolyFlags);
  }
  else
  {
    m_HLRenderer.OnDrawUI(Frame, Info, X, Y, XL, YL, U, V, UL, VL, Span, Z, Color, Fog, PolyFlags);
  }
}

void UD3D9FPRenderDevice::Draw2DLine(FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector P1, FVector P2)
{
  assert(false);
}

void UD3D9FPRenderDevice::Draw2DPoint(FSceneNode* Frame, FPlane Color, DWORD LineFlags, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, FLOAT Z)
{
  assert(false);
}

void UD3D9FPRenderDevice::ClearZ(FSceneNode* Frame)
{
  m_LLRenderer.ClearDepth();
}

void UD3D9FPRenderDevice::PushHit(const BYTE* Data, INT Count)
{
  if (Count >= 4)
  {
    m_HLRenderer.PushRenderObject(Data, Count);
  }
}

void UD3D9FPRenderDevice::PopHit(INT Count, UBOOL bForce)
{
  if (Count >= 4)
  {
    m_HLRenderer.PopRenderObject(Count);
  }
}

/**
Something to do with FPS counters etc, not needed.
*/
void UD3D9FPRenderDevice::GetStats(TCHAR* Result)
{

}

void UD3D9FPRenderDevice::ReadPixels(FColor* Pixels)
{
  GLog->Log(L"Dumping screenshot...");
  //m_LLRenderer.getScreenshot((m_LLRenderer.Vec4_byte*)Pixels);
  GLog->Log(L"Not implemented :-(");
  //TODO: look how we can get the contents after NVRemix is done...
}

UBOOL UD3D9FPRenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
  GLog->Logf(L"Command: %s\n", Cmd);
  if (!g_options.renderingDisabled)
  {
    //First try parent
    wchar_t* ptr;
    if (URenderDevice::Exec(Cmd, Ar))
    {
      return 1;
    }
    else if (ParseCommand(&Cmd, L"GetRes"))
    {
      std::wstring displayModeString;

      auto count = 0;
      auto displayModes = m_LLRenderer.GetDisplayModes();
      std::vector<D3DDISPLAYMODE> processedDisplayModes;

      for (const auto& m : displayModes)
      {
        const bool alreadyRegistered = std::any_of(processedDisplayModes.begin(), processedDisplayModes.end(), 
          [&](const D3DDISPLAYMODE& pComp) {
            return pComp.Width == m.Width && pComp.Height == m.Height;
          });

        if (!alreadyRegistered)
        {
          wchar_t tmpString[100]{ 0 };
          swprintf_s(tmpString, L"%dx%d", m.Width, m.Height);
          displayModeString += tmpString;
          processedDisplayModes.push_back(m);

          if (count++ >= 16)
          {
            break;
          }
          else
          {
            displayModeString += L" ";
          }
        }
      }

      if (!displayModeString.empty())
      {
        Ar.Log(displayModeString.c_str());
      }
      return 1;
    }
    else if (ParseCommand(&Cmd, L"SetRes"))
    {
      if (m_CurrentResColorBytes && m_CurrentResFullscreen)
      {
        INT X=appAtoi(Cmd);
        TCHAR* CmdTemp = appStrchr(Cmd,'x') ? appStrchr(Cmd,'x')+1 : appStrchr(Cmd,'X') ? appStrchr(Cmd,'X')+1 : TEXT("");
        INT Y=appAtoi(CmdTemp);
        SetRes(X, Y, *m_CurrentResColorBytes, *m_CurrentResFullscreen);
      }
    }
    else if ((ptr = (wchar_t*)wcswcs(Cmd, L"Brightness"))) //Brightness is sent as "brightness [val]".
    {
      if ((ptr = wcschr(ptr, ' ')))//Search for space after 'brightness'
      {
        float b;
        b = _wtof(ptr); //Get brightness value;
        GLog->Logf(L"[EchelonRenderer]\t Setting brightness: %f (NOT IMPLEMENTED)", b);
        //TODO:
        //m_LLRenderer.setBrightness(b);
      }
    }
    return 0;
  }
  else
  {
    if (URenderDevice::Exec(Cmd, Ar))
    {
      return 1;
    }
    return 0;
  }
}

void UD3D9FPRenderDevice::SetSceneNode(FSceneNode* Frame)
{
}

std::unordered_set<void*> _processed;
std::unordered_set<void*> _seen;
void UD3D9FPRenderDevice::PrecacheTexture(FTextureInfo& Info, DWORD PolyFlags)
{
  if (auto it = _processed.insert(Info.Texture); it.second)
  {
    return;
  }

  bool isAnimated = Info.Texture->AnimNext != nullptr && Info.Texture->AnimNext != Info.Texture;
  if (isAnimated)
  {
    if (auto it = _seen.insert(Info.Texture->AnimNext); it.second)
    {
      int x = 1;
    }
  }
}

void UD3D9FPRenderDevice::EndFlash()
{
}

void UD3D9FPRenderDevice::OnLevelChange()
{
  _seen.clear();
  _processed.clear();
  m_NotifyLevelHasChanged = true;
}

///

UBenchmark::UBenchmark()
{
  static bool initialized = false;
  if (!initialized)
  {
    InitializeEchelonCore();
    InstallUGameEngineHooks();
    
    g_CommandManager.Initialize();
    g_DemoManager.Initialize();

  }
  g_DemoManager.RunBenchmark(true, false, 1000);
}