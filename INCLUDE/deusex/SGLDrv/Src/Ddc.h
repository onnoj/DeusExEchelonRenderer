//******************************************************************************
// Name:      DDC.H
// Title:     Header file for Direct Draw Control		
// Author(s): Jayeson Lee-Steere
// Created :  97/08/08	
//            98/01/14 - Converted to C++
//******************************************************************************
#ifndef DDC_H
#define DDC_H

#include <ddraw.h>
#include "hooksgl.h"

// Error codes
enum
{
	DDCERR_OK=0,

	DDCERR_FAILED_TO_CREATE_DDRAW_OBJECT,
	DDCERR_FAILED_TO_SET_DDRAW_COOPERATIVE_LEVEL,
	DDCERR_FAILED_TO_SET_DDRAW_VIDEO_MODE,
	DDCERR_FAILED_TO_CREATE_DDRAW_SURFACE,
	DDCERR_FAILED_TO_GET_DDRAW_ATTACHED_SURFACE,
	DDCERR_FAILED_TO_CREATE_DDRAW_CLIPPER,
	DDCERR_FAILED_TO_SET_DDRAW_CLIPPER_WINDOW,
	DDCERR_FAILED_TO_SET_DDRAW_SURFACE_CLIPPER,
	DDCERR_FAILED_TO_GET_DDRAW_PIXEL_FORMAT,
	DDCERR_PIXEL_FORMAT_IS_INVALID,
	DDCERR_FAILED_TO_CREATE_SGL_SCREEN_DEVICE,
	DDCERR_FAILED_TO_LOAD_DDRAW_DLL,
	DDCERR_FAILED_TO_FIND_CREATE_FUNCTION
};

typedef HRESULT (WINAPI *DD_CREATE_FUNC)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

class UDDC : public USGL
{
	// Init/Deinit stuff
	HINSTANCE ddInstance;	
	DD_CREATE_FUNC ddCreateFunc;
	IDirectDraw *dd1;			// DirectDraw object
	IDirectDraw2* dd;			// DirectDraw2 object
	PROC_2D_CALLBACK Proc2D;	// Callback for doing 2D stuff
	sgl_int32 iStrictLocks;

	// Rendering object stuff
	BOOL bHaveRenderingObject;
	BOOL bAppActive;
	HWND TargetWindow;
	WNDPROC TargetWindowProc;
	BOOL bBlockMessages;
	BOOL bHaveTimer;
	BOOL bFullScreen;
	int TargetWidth,TargetHeight,RealBpp,NumBuffers;
	RECT WindowRect;				// Used only in windowed mode

	IDirectDrawSurface *FrontBuffer;
	IDirectDrawSurface *BackBuffer;
//	IDirectDrawSurface *BackBackBuffer;
	IDirectDrawClipper *Clipper;
	
	int SglScreenDevice;
	BOOL bInFrame;					// In between sgltri_startofframe/sgltri_render
	BOOL bRenderTargetLocked;		// This indicates the render target has been locked
									// because strict locks is set
	BOOL bFlipPending;
	int FlipTimeoutCount;

public:
	char *DDCGetErrorMessage(int ErrorMessage);

	int DDCInit(PROC_2D_CALLBACK Proc2d=NULL);
	int DDCShutdown();
	int DDCCreateRenderingObject(HWND TargetWindow,BOOL FullScreen,
								 int FullScreenWidth,int FullScreenHeight,
								 int FullScreenBpp,int FullScreenNumBuffers);
	int DDCDestroyRenderingObject();

	friend int _DDCNextAddressProc(P_CALLBACK_ADDRESS_PARAMS pParamBlk);
	friend int _DDCEORProc(void);
	friend LRESULT CALLBACK _DDCWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	void _DDCClearSurface(IDirectDrawSurface *Surface);
	BOOL _DDCRestoreSurfacesIfLost();

	int DDCGetRenderingObjectWidth();
	int DDCGetRenderingObjectHeight();
	int DDCGetRenderingObjectXOffset();
	int DDCGetRenderingObjectYOffset();
	int DDCGetRenderingObjectBpp();
	int DDCGetRenderingObjectRealBpp();
	int DDCGetRenderingObjectNumBuffers();

	IDirectDrawSurface *DDCGetRenderingObjectFrontBuffer(BOOL bWaitForFlip);
	IDirectDrawSurface *DDCGetRenderingObjectBufferForScreenshot(BOOL bWaitForFlip);

	void DDCStartOfFrame();
	void DDCRender();
	BOOL DDCIsRenderActive();
	void DDCWaitForRenderToComplete(ULONG Timeout);
	void DDCFlip();
};

#endif // DDC_H