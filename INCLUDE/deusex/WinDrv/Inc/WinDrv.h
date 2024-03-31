/*=============================================================================
	WinDrvPrivate.cpp: Unreal Windows viewport and platform driver.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

#ifndef _INC_WINDRV
#define _INC_WINDRV

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/

#ifndef WINDRV_API
	#define WINDRV_API DLL_IMPORT
#endif

/*----------------------------------------------------------------------------
	Dependencies.
----------------------------------------------------------------------------*/

// Windows includes.
#pragma warning( disable : 4201 )
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <ddraw.h>
#include <dinput.h>
#include "Res\WinDrvRes.h"

// Unreal includes.
#include "Engine.h"
#include "Window.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Declarations.
-----------------------------------------------------------------------------*/

// Classes.
class UWindowsViewport;
class UWindowsClient;

// Global functions.
WINDRV_API TCHAR* ddError( HRESULT Result );
WINDRV_API TCHAR* diError( HRESULT Result );

/*-----------------------------------------------------------------------------
	UWindowsClient.
-----------------------------------------------------------------------------*/

//
// Windows implementation of the client.
//
class DLL_EXPORT UWindowsClient : public UClient, public FNotifyHook
{
	DECLARE_CLASS(UWindowsClient,UClient,CLASS_Transient|CLASS_Config)

	// DirectDraw Constants.
	typedef HRESULT (WINAPI *DD_CREATE_FUNC)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
	typedef HRESULT (WINAPI *DD_ENUM_FUNC  )(LPDDENUMCALLBACKA lpCallback,LPVOID lpContext);

	// DirectInput Constants.
	typedef HRESULT (WINAPI *DI_CREATE_FUNC)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT *lplpDirectInput, LPUNKNOWN punkOuter);
 

	// Configuration.
	BITFIELD			UseDirectDraw;
	BITFIELD			UseDirectInput;
	BITFIELD			UseJoystick;
	BITFIELD			StartupFullscreen;
	BITFIELD			SlowVideoBuffering;
	BITFIELD			DeadZoneXYZ;
	BITFIELD			DeadZoneRUV;
	BITFIELD			InvertVertical;
	FLOAT				ScaleXYZ;
	FLOAT				ScaleRUV;

	// Variables.
	HDC					hMemScreenDC;
	UBOOL				InMenuLoop;
	UBOOL				ConfigReturnFullscreen;
	INT					NormalMouseInfo[3];
	INT					CaptureMouseInfo[3];
	IDirectDraw2*		dd;
	DD_CREATE_FUNC		ddCreateFunc;
	DD_ENUM_FUNC		ddEnumFunc;

	IDirectInput*		di;
	DI_CREATE_FUNC		diCreateFunc;

	WConfigProperties*	ConfigProperties;
	TArray<FVector>		DirectDrawModes[MAX_COLOR_BYTES];
	TArray<FGuid>		DirectDrawGUIDs;
	ATOM				hkAltEsc, hkAltTab, hkCtrlEsc, hkCtrlTab;
	JOYCAPSA			JoyCaps;

	// Constructors.
	UWindowsClient();
	void StaticConstructor();

	// FNotifyHook interface.
	void NotifyDestroy( void* Src );

	// UObject interface.
	void Destroy();
	void PostEditChange();
	void ShutdownAfterError();

	// UClient interface.
	void Init( UEngine* InEngine );
	void ShowViewportWindows( DWORD ShowFlags, INT DoShow );
	void EnableViewportWindows( DWORD ShowFlags, INT DoEnable );
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	void Tick();
	void MakeCurrent( UViewport* InViewport );
	class UViewport* NewViewport( const FName Name );

	// UWindowsClient interface.
	void ddEndMode();
	void diShutdown();

	// Static functions.
	static HRESULT WINAPI EnumModesCallback( DDSURFACEDESC* SurfaceDesc, void* Context );
	static BOOL WINAPI EnumDriversCallbackA( GUID* GUID, ANSICHAR* DriverDescription, ANSICHAR* DriverName, void* Context );
};

/*-----------------------------------------------------------------------------
	UWindowsViewport.
-----------------------------------------------------------------------------*/

//
// Viewport window status.
//
enum EWinViewportStatus
{
	WIN_ViewportOpening	= 0, // Viewport is opening and hWnd is still unknown.
	WIN_ViewportNormal	= 1, // Viewport is operating normally, hWnd is known.
	WIN_ViewportClosing	= 2, // Viewport is closing and CloseViewport has been called.
};

//
// A Windows viewport.
//
class DLL_EXPORT UWindowsViewport : public UViewport
{
	DECLARE_CLASS(UWindowsViewport,UViewport,CLASS_Transient)
	DECLARE_WITHIN(UWindowsClient)

	// Variables.
	class WWindowsViewportWindow* Window;
	EWinViewportStatus  Status;
	HBITMAP				hBitmap;
	HMENU				hMenu;
	HWND				ParentWindow;
	IDirectDrawSurface*	ddFrontBuffer;
	IDirectDrawSurface*	ddBackBuffer;
	DDSURFACEDESC 		ddSurfaceDesc;
	HHOOK				hHook;
	INT					HoldCount;
	DWORD				BlitFlags;
	UBOOL				Borderless;
	UBOOL				DirectDrawMinimized;
	LPDIRECTINPUTDEVICE diKeyboard;
	LPDIRECTINPUTDEVICE diMouse;
	DIDEVICEOBJECTDATA	*diMouseBuffer;
	DIMOUSESTATE		diMouseState;
	INT					diOldMouseX;
	INT					diOldMouseY;
	INT					diOldMouseZ;
	UBOOL				diLMouseDown;
	UBOOL				diRMouseDown;
	UBOOL				diMMouseDown;
	UBOOL				AcquiredDIMouse;

	// Info saved during captures and fullscreen sessions.
	POINT				SavedCursor;
	FRect				SavedWindowRect;
	INT					SavedColorBytes;
	INT					SavedCaps;
	INT					DesktopColorBytes;
	HCURSOR				StandardCursors[10];

	// Constructor.
	UWindowsViewport();

	// UObject interface.
	void Destroy();
	void ShutdownAfterError();

	// UViewport interface.
	UBOOL Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData=NULL, INT* HitSize=0 );
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
	UBOOL ResizeViewport( DWORD BlitFlags, INT NewX=INDEX_NONE, INT NewY=INDEX_NONE, INT NewColorBytes=INDEX_NONE );
	UBOOL IsFullscreen();
	void Unlock( UBOOL Blit );
	void Repaint( UBOOL Blit );
	void SetModeCursor();
	void UpdateWindowFrame();
	void OpenWindow( DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY );
	void CloseWindow();
	void UpdateInput( UBOOL Reset );
	void* GetWindow();
	void SetMouseCapture( UBOOL Capture, UBOOL Clip, UBOOL FocusOnly );
	void AcquireDIMouse( UBOOL Capture );

	// UWindowsViewport interface.
	LONG ViewportWndProc( UINT Message, UINT wParam, LONG lParam );
	void ToggleFullscreen();
	void EndFullscreen();
	void FindAvailableModes();
	void TryHardware3D( UWindowsViewport* Viewport );
	void SetTopness();
	UBOOL CauseInputEvent( INT iKey, EInputAction Action, FLOAT Delta=0.0 );
	UBOOL JoystickInputEvent( FLOAT Delta, EInputKey Key, FLOAT Scale, UBOOL DeadZone );
	UBOOL ddSetMode( INT Width, INT Height, INT ColorBytes );
	UBOOL diSetupKeyboardMouse();
	void diShutdownKeyboardMouse();
	void TryRenderDevice( const TCHAR* ClassName, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );

	// Static functions.
	static LRESULT CALLBACK KeyboardProc( INT Code, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK SysMsgProc( INT nCode, WPARAM wParam, LPARAM lParam );
};

//
// A windows viewport window.
//
class DLL_EXPORT WWindowsViewportWindow : public WWindow
{
	W_DECLARE_CLASS(WWindowsViewportWindow,WWindow,CLASS_Transient)
	DECLARE_WINDOWCLASS(WWindowsViewportWindow,WWindow,Window)
	class UWindowsViewport* Viewport;
	WWindowsViewportWindow()
	{}
	WWindowsViewportWindow( class UWindowsViewport* InViewport )
	: Viewport( InViewport )
	{}
	LONG WndProc( UINT Message, UINT wParam, LONG lParam )
	{
		return Viewport->ViewportWndProc( Message, wParam, lParam );
	}
};

#endif //_INC_WINDRV
/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
