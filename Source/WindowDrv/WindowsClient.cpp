#include "DeusExEchelonRendererWindowDrv_PCH.h"
#pragma hdrstop

#include <SDL.h>
#include <SDL_syswm.h>

#include "WindowsClient.h"
#include "WindowsViewport.h"

UWindowsClient::UWindowsClient()
{
  // This constructor used to initialize the UWindowsClient class.
  // It would set up hotkeys using `GlobalAddAtom` for various key combinations like Alt+Esc, Alt+Tab, etc.
  // The `di` variable, presumably used for DirectInput, was initialized to NULL here.
}

void UWindowsClient::StaticConstructor()
{
  // This function used to be a static constructor, responsible for defining configuration properties
  // related to display and joystick settings. It added boolean and float properties using macros such as `UBoolProperty` and `UFloatProperty`.

  new(GetClass(),TEXT("StartupFullscreen"), RF_Public)UBoolProperty(CPP_PROPERTY(m_StartupFullscreen), TEXT("Display"),  CPF_Config );
}

void UWindowsClient::Init(UEngine* InEngine) 
{
  // This function used to initialize the platform-specific viewport manager for Windows.
  // It called the base class initializer and registered the window class.
  // The function also created a working device context and captured mouse information using `SystemParametersInfoX`.
  // It attempted to initialize DirectDraw, listing drivers, creating DirectDraw, and enumerating display modes.
  // Additionally, it would try to initialize DirectInput if enabled, and set some 3dfx environment variables.


  UClient::Init( InEngine );
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    GError->Log(L"Failed to initialize SDL");
    UObject::StaticShutdownAfterError();
    return;
  }

  SDL_VERSION(&m_SDLInfo.version);

  //Resolution
  GConfig->GetBool(L"WinDrv.WindowsClient", L"StartupFullscreen", m_StartupFullscreen);

  GConfig->GetInt(L"WinDrv.WindowsClient", L"FullscreenViewportX", FullscreenViewportX);
  GConfig->GetInt(L"WinDrv.WindowsClient", L"FullscreenViewportY", FullscreenViewportY);
  GConfig->GetInt(L"WinDrv.WindowsClient", L"WindowedViewportX", WindowedViewportX);
  GConfig->GetInt(L"WinDrv.WindowsClient", L"WindowedViewportY", WindowedViewportY);

  GConfig->GetInt(L"WinDrv.WindowsClient",L"WindowedViewportX", WindowedViewportX     );
  GConfig->GetInt(L"WinDrv.WindowsClient",L"WindowedViewportY", WindowedViewportY     );
  GConfig->GetInt(L"WinDrv.WindowsClient",L"WindowedColorBits", WindowedColorBits     );
  GConfig->GetInt(L"WinDrv.WindowsClient",L"FullscreenViewportX", FullscreenViewportX   );
  GConfig->GetInt(L"WinDrv.WindowsClient",L"FullscreenViewportY", FullscreenViewportY	);
  GConfig->GetInt(L"WinDrv.WindowsClient",L"FullscreenColorBits", FullscreenColorBits	);
  GConfig->GetFloat(L"WinDrv.WindowsClient",L"MipFactor", MipFactor				);
  GConfig->GetFloat(L"WinDrv.WindowsClient",L"Brightness", Brightness			);
  GConfig->GetInt(L"WinDrv.WindowsClient",L"TextureDetail", TextureLODSet[1]);
  GConfig->GetInt(L"WinDrv.WindowsClient",L"SkinDetail", TextureLODSet[2]);
  GConfig->GetFloat(L"WinDrv.WindowsClient",L"MinDesiredFrameRate", MinDesiredFrameRate	);

  auto GConfigGetBitBool = [&](const TCHAR* Section, const TCHAR* Key, BITFIELD& Value, const TCHAR* Filename = NULL)
    {
      UBOOL tmp;
      GConfig->GetBool(Section, Key, tmp, Filename);
      Value = tmp;
    };
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"CaptureMouse", CaptureMouse);
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"CurvedSurfaces", CurvedSurfaces);
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"ScreenFlashes", ScreenFlashes);
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"NoLighting", NoLighting);
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"Decals", Decals);
  GConfigGetBitBool(L"WinDrv.WindowsClient", L"NoDynamicLights", NoDynamicLights);

  //IMPLEMENT_WINDOWCLASS(WWindowsViewportWindow,GIsEditor ? (CS_DBLCLKS|CS_OWNDC) : (CS_OWNDC));

  PostEditChange();
}

void UWindowsClient::NotifyDestroy(void* Src) 
{
  // This function used to be called when an object was being destroyed.
  // If the object being destroyed was a configuration property window,
  // it would clear the reference and potentially toggle fullscreen mode in the viewport.

}

void UWindowsClient::Destroy() 
{
  // This function used to shut down the Windows viewport manager subsystem.
  // It ensured DirectInput was shut down and released any DirectDraw resources.
  // Additionally, it cleaned up Windows-specific resources such as device contexts,
  // and reset mouse parameters.
  SDL_Quit();
}

void UWindowsClient::ShutdownAfterError() 
{
  // This function used to handle shutdown in the event of a critical error.
  // It would restore mouse settings, show the cursor, shut down audio if available,
  // and ensure all viewports were closed down properly.
}

void UWindowsClient::PostEditChange() 
{
  // This function used to handle changes in the configuration properties.
  // If a joystick was enabled, it would check for its availability and log the result.

  Super::PostEditChange();
}



void UWindowsClient::ShowViewportWindows(DWORD ShowFlags, INT DoShow) 
{
  // This function used to show or hide all viewport windows with matching ShowFlags.
  // If ShowFlags was 0, it applied the action to all viewports.
}

void UWindowsClient::EnableViewportWindows(DWORD ShowFlags, INT DoEnable) 
{
  // This function used to enable or disable all viewport windows with matching ShowFlags.
  // If ShowFlags was 0, it affected all viewports.
}

UBOOL UWindowsClient::Exec(const TCHAR* Cmd, FOutputDevice& Ar) 
{ 
  // This function used to execute console commands passed as arguments (`Cmd`).
  // It first called the parent class’s `Exec` function to handle any standard commands.

  return Super::Exec(Cmd, Ar); 
}

void UWindowsClient::Tick() 
{
  // This function used to handle the per-frame processing (or "ticking") of the viewports.
  // It would iterate through all viewports and repaint those that needed updating,
  // specifically prioritizing the one that hadn't been updated in the longest time.
  for (INT i = 0; i < Viewports.Num(); i++)
  {
    this->Engine->Draw(Viewports(i), 1);
  }
}

void UWindowsClient::MakeCurrent(UViewport* InViewport) 
{
  // This function used to make a given viewport the current active viewport.
  // If the input `InViewport` was NULL, it would deselect any current viewport.
  // It ensured that only one viewport was active at a time.
}

UViewport* UWindowsClient::NewViewport(const FName Name) 
{ 
  // This function used to create a new viewport object with the given `Name`.
  // It returned an instance of `UWindowsViewport`, linking it to this client.

  return new( this, Name )UWindowsViewport;
}
