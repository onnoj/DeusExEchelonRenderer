#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop
#include "uefacade.h"

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
#include "utils/debugmenu.h"
#include "utils/configmanager.h"
#include "hacks/hacks.h"
#include "hacks/misc.h"
IMPLEMENT_PACKAGE(DeusExEchelonRenderer);
IMPLEMENT_CLASS(UD3D9FPRenderDevice);

LowlevelRenderer UD3D9FPRenderDevice::m_LLRenderer;
HighlevelRenderer UD3D9FPRenderDevice::m_HLRenderer;
std::mutex UD3D9FPRenderDevice::m_Lock;

class FLogOutImpl : public FOutputDevice
{
public:
	void Serialize( const TCHAR* V, EName Event )
	{
		char b[1024]{};
		WideCharToMultiByte(CP_ACP, 0, V, -1, &b[0], std::size(b), NULL, NULL);
		printf("[%d] %s\n", int(Event), b);
	}
} LogOutImpl;

class FErrOutImpl : public FOutputDeviceError
{
public:
	void Serialize( const TCHAR* V, EName Event )
	{
		char b[1024]{};
		WideCharToMultiByte(CP_ACP, 0, V, -1, &b[0], std::size(b), NULL, NULL);
		printf("[NORMALLY-FATAL][%d] %s\n", int(Event), b);
	}
	void HandleError() {};
} ErrOutImpl;

void UD3D9FPRenderDevice::StaticConstructor()
{
	if (!g_options.renderingDisabled)
	{
		//Create a console to print debug stuff to.
#if !defined(RELEASE_CONFIG)
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		printf("Hello world\n");

		GNull = &LogOutImpl;
		GLog = &LogOutImpl;
		GError = &ErrOutImpl;
#endif
	}
}

UD3D9FPRenderDevice::UD3D9FPRenderDevice()
{
	//I don't know why, but, we _need_ to declare a constructor here.
	//default constructor somehow leads to problems.
}

UBOOL UD3D9FPRenderDevice::Init(UViewport* pInViewport,int32_t pWidth, int32_t pHeight, int32_t pColorBytes, UBOOL pFullscreen)
{
  std::unique_lock lock(m_Lock);
  
	if (GetClass() == nullptr)
	{
		return FALSE;
	}

	g_ConfigManager.LoadConfig();
	g_ConfigManager.SaveConfig(); //save file again so that we have the defaults

	InstallRTXConfigPatches();
	InstallBytePatches();
	InstallThreadAffinityHacks();
	InstallFSceneNodeHacks();
	InstallFSpanBufferHacks();
	InstallURenderHacks();
	InstallUMeshHacks();
	InstallUGameEngineHacks();
	InstallFFDynamicSpriteHacks();
	InstallFDynamicItemFilterHacks();
	InstallULightManagerHacks();

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
	UD3D9FPRenderDevice::Viewport->ResizeViewport(BLIT_HardwarePaint | BLIT_Direct3D);
	UBOOL Result = UD3D9FPRenderDevice::Viewport->ResizeViewport(blitType, pWidth, pHeight, pColorBytes);
	if (!Result) {
		GError->Logf(L"[EchelonRenderer]\t Unreal Engine failed to resize the viewport with parameters {type:%x, width:%d, height:%d, color:%x}", blitType, pWidth, pHeight, pColorBytes);
		return 0;
	}

	bool rendererInitialized = false;
	for (int i = 0; i < 5; i++)
	{
		if (!m_LLRenderer.Initialize((HWND)pInViewport->GetWindow(), pWidth, pHeight, pColorBytes, pFullscreen))
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

	if(!this->SetRes(pWidth,pHeight,pColorBytes,pFullscreen))
	{
		GError->Log(L"SetRes failed during initialization.");
		return false;
	}

	m_HLRenderer.Initialize(&m_LLRenderer);
	
	//force lights off if running with remix disabled
	g_options.hasLights = g_options.hasLights && Misc::IsNvRemixAttached(true);

	//Other renderers solve a compatibility issue by setting the processor affinity to a single-cpu
	//Presumably to solve an issue with __rdtsc drift. For us, this screws with NV Remix.
	//If this is an issue, we can set thread affinity for Deus Ex's main thread and the rendering thread, instead.
	//SetProcessAffinityMask(GetCurrentProcess(),0x1); 
	//Misc::applyThreadAffinityMasks();

	auto m = GWindowManager;

	return 1;
}

void UD3D9FPRenderDevice::Exit()
{
  std::unique_lock lock(m_Lock);
	GLog->Log(L"[EchelonRenderer]\t Direct3D 9 Fixed-Function Pipeline renderer exiting.");
	g_DebugMenu.Shutdown();
	m_HLRenderer.Shutdown();
	m_LLRenderer.Shutdown();
#if !defined(RELEASE_CONFIG)
	FreeConsole();
#endif
	UninstallULightManagerHacks();
	UninstallFDynamicItemFilterHacks();
	UninstallFFDynamicSpriteHacks();
	UninstallURenderHacks();
	UninstallUMeshHacks();
	UninstallFSpanBufferHacks();
	UninstallFSceneNodeHacks();
	UninstallThreadAffinityHacks();
	UninstallUGameEngineHacks();
	UninstallBytePatches();
}

UBOOL UD3D9FPRenderDevice::SetRes(INT pWidth, INT pHeight, INT pColorBytes, UBOOL pFullscreen)
{
	static INT lastWidth = pWidth;
	static INT lastHeight = pHeight;

	if (pWidth == 0 || pHeight == 0)
	{
		pFullscreen = 0;
		pWidth = lastWidth;
		pHeight = lastHeight;
	}

	if (pFullscreen)
	{
    if (auto res = m_LLRenderer.FindClosestResolution(pWidth, pHeight); res)
    {
      GLog->Logf(L"[EchelonRenderer]\t Fullscreen Setres was requested, and matched %dx%d to available resolution %dx%d.", pWidth, pHeight, res->Width, res->Height);
      pWidth = res->Width;
      pHeight = res->Height;
    }
    else
    {
      pFullscreen = false;
      GLog->Logf(L"[EchelonRenderer]\t SetRes failed, could not find matching fullscreen resolution for %dx%d, possible device returned no modes. Swithing to windowed.", pWidth, pHeight);
    }
	}

	const auto blitType = (pFullscreen ? (BLIT_Fullscreen | BLIT_Direct3D) : (BLIT_HardwarePaint | BLIT_Direct3D));
	UBOOL Result = URenderDevice::Viewport->ResizeViewport(blitType, pWidth, pHeight, pColorBytes);
	if (!Result) 
	{
		GError->Log(L"SetRes failed: Error resizing viewport.");
		return 0;
	}	
	if(!m_LLRenderer.ResizeDisplaySurface(0, 0, pWidth,pHeight,(pFullscreen!=0)))
	{
		GError->Log(L"SetRes failed due to internal error in m_LLRenderer.Resize()");
		return 0;
	}
#if 0
	//We need to calculate our FoV. Default is 75.
	//customFOV = Misc::getFov(defaultFOV,Viewport->SizeX,Viewport->SizeY);
	//Does this need to be set on Viewport->Actor->FovAngle?
#endif
	Result = m_LLRenderer.ResizeDisplaySurface(0, 0, pWidth, pHeight, pFullscreen);
	if (Result)
	{
		lastWidth = pWidth;
		lastHeight = pHeight;
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
void UD3D9FPRenderDevice::Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* InHitData, INT* InHitSize )
{
	renderStart = std::chrono::high_resolution_clock::now();
	if (!g_options.renderingDisabled)
	{
		static uint32_t lastHash = 0;
		static uint32_t frameIdx = 0;
		char fileName[255]{ 0 };

		//imGUI
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
		if (now > lastUpdate + std::chrono::milliseconds(1000*10))
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
}

void UD3D9FPRenderDevice::Tick(FLOAT DeltaTime)
{
	g_DebugMenu.Render();
}

void UD3D9FPRenderDevice::DrawGouraudPolygon( FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span )
{
	m_HLRenderer.OnDrawMesh(Frame, Info, Pts, NumPts, PolyFlags, Span);
}

void UD3D9FPRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	m_HLRenderer.OnDrawGeometry(Frame, Surface, Facet);
}

void UD3D9FPRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, class FSpanBuffer* Span, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags)
{
	std::wstring textureName = Info.Texture->GetFullName();
	if (textureName.find(L"Corona") != -1)
	{
		constexpr float scale = 0.5f;
		constexpr float half = 0.5f;
		const float dx = XL - (XL * scale);
		const float dy = YL - (YL * scale);
		X += (dx*half);
		Y += (dy*half);
		XL *= scale;
		YL *= scale;
	}
  m_HLRenderer.OnDrawUI(Frame, Info, X, Y, XL, YL, U, V, UL, VL, Span, Z, Color, Fog, PolyFlags);
}

void UD3D9FPRenderDevice::Draw2DLine( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector P1, FVector P2 )
{
	assert(false);
}

void UD3D9FPRenderDevice::Draw2DPoint( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, FLOAT Z )
{
	assert(false);
}

void UD3D9FPRenderDevice::ClearZ( FSceneNode* Frame )
{
	m_LLRenderer.ClearDepth();
}

void UD3D9FPRenderDevice::PushHit( const BYTE* Data, INT Count )
{
	assert(false);
}

void UD3D9FPRenderDevice::PopHit( INT Count, UBOOL bForce )
{
	assert(false);
}

/**
Something to do with FPS counters etc, not needed. 
*/
void UD3D9FPRenderDevice::GetStats( TCHAR* Result )
{

}

void UD3D9FPRenderDevice::ReadPixels( FColor* Pixels )
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

      for (const auto& m : displayModes)
      {
        wchar_t tmpString[100]{ 0 };
        swprintf_s(tmpString, L"%dx%d", m.Width, m.Height);
        displayModeString += tmpString;
        
        if (count++ >= 16)
        {
          break;
        }
        else
        {
          displayModeString += L" ";
        }
      }

      if (!displayModeString.empty())
      {
        Ar.Log(displayModeString.c_str());
      }
			return 1;
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

void UD3D9FPRenderDevice::SetSceneNode(FSceneNode* Frame )
{	
}

void UD3D9FPRenderDevice::PrecacheTexture( FTextureInfo& Info, DWORD PolyFlags )
{
}

void  UD3D9FPRenderDevice::EndFlash()
{
}
