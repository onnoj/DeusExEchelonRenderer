// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtRoot.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal root window extension
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_ROOT_H_
#define _EXT_ROOT_H_


// ----------------------------------------------------------------------
// XRootWindow

class EXTENSION_API XRootWindow : public XModalWindow
{
	DECLARE_CLASS(XRootWindow, XModalWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XRootWindow)

	friend class XWindow;
	friend class XTabGroupWindow;
	friend class XModalWindow;
	friend class XViewportWindow;

	public:
		XRootWindow(APlayerPawnExt *parentPawn);

		// Structors
		virtual void Init(APlayerPawnExt *parentPawn);
		void Init(XWindow *newParent) { debugf(TEXT("Root cannot have a parent window!")); }
		void CleanUp(void);
		void Destroy(void);

	protected:
		// PlayerPawn this root window is associated with
		APlayerPawnExt *parentPawn;

		// Next root window in the global root window list
		XRootWindow    *nextRootWindow;

		// Cursor position info
		FLOAT          mouseX;                    // Cursor X pos
		FLOAT          mouseY;                    // Cursor Y pos
		FLOAT          prevMouseX;                // Last cursor X pos
		FLOAT          prevMouseY;                // Last cursor Y pos
		XWindow        *lastMouseWindow;          // Last window the cursor was in
		BITFIELD       bMouseMoved:1 GCC_PACK(4); // TRUE if the mouse moved
		BITFIELD       bMouseMoveLocked:1;        // TRUE if mouse movement is disabled
		BITFIELD       bMouseButtonLocked:1;      // TRUE if mouse buttons are disabled
		BITFIELD       bCursorVisible:1;          // TRUE if the cursor is visible

	public:
		// Default cursors
		XCursor        *defaultEditCursor GCC_PACK(4); // Cursor for edit widgets
		XCursor        *defaultMoveCursor;             // General movement cursor
		XCursor        *defaultHorizontalMoveCursor;   // Horizontal movement cursor
		XCursor        *defaultVerticalMoveCursor;     // Vertical movement cursor
		XCursor        *defaultTopLeftMoveCursor;      // Upper left to lower right cursor
		XCursor        *defaultTopRightMoveCursor;     // Upper right to lower left cursor

		// Sound options
		BITFIELD       bPositionalSound:1 GCC_PACK(4); // TRUE if positional sound is enabled

	protected:
		// Input windows
		XWindow        *grabbedWindow GCC_PACK(4);     // Recipient window for all mouse events
		XWindow        *focusWindow;                   // Recipient window for all keyboard events

		// Input handling reference counters
		INT            handleMouseRef;    // Should root handle mouse events?
		INT            handleKeyboardRef; // Should root handle keyboard events?

		// Initialization reference counter
		INT            initCount;         // Number of windows to be initialized this tick

		// Rendered area information
		BITFIELD       bRender:1 GCC_PACK(4);   // TRUE if 3D areas should be rendered
		BITFIELD       bClipRender:1;           // TRUE if the 3D area is clipped
		BITFIELD       bStretchRawBackground:1;	// TRUE if raw background should be stretched
		FLOAT          renderX GCC_PACK(4);     // X offset of rendered area
		FLOAT          renderY;                 // Y offset of rendered area
		FLOAT          renderWidth;             // Width of rendered area
		FLOAT          renderHeight;            // Height of rendered area
		UTexture       *rawBackground;          // Background graphic drawn in unrendered areas
		FLOAT          rawBackgroundWidth;      // Width of background graphic
		FLOAT          rawBackgroundHeight;     // Height of background graphic
		FColor         rawColor;                // Color of raw background texture

		// Statistical variables
		INT            tickCycles;                // Number of cycles used during windows tick
		INT            paintCycles;               // Number of cycles used during PaintWindows call
		BITFIELD       bShowStats:1 GCC_PACK(4);  // Should statistics be shown on root window?
		BITFIELD       bShowFrames:1;             // Should we draw debugging frames around all windows?
		UTexture       *debugTexture GCC_PACK(4); // Debugging texture
		FLOAT          frameTimer;                // Timer used for frames

		// Button click information used to determine double clicks
		FLOAT         multiClickTimeout;  // Max amount of time between multiple button clicks
		FLOAT         maxMouseDist;       // Maximum mouse distance for multi-click to work
		INT           clickCount;         // Current click number (zero-based)
		INT           lastButtonType;     // Last mouse button handled
		FLOAT         lastButtonPress;    // Time remaining for last button press
		XWindow       *lastButtonWindow;  // Last window clicked in
		FLOAT         firstButtonMouseX;  // X position of initial button press
		FLOAT         firstButtonMouseY;  // Y position of initial button press

		// List of all current key states
		BYTE          keyDownMap[IK_MAX]; // TRUE if pressed, FALSE if not

		// Multipliers
		INT           hMultiplier;        // Horizontal multiplier
		INT           vMultiplier;        // Vertical multiplier

		// Snapshot-related variables
		INT           snapshotWidth;      // Snapshot width
		INT           snapshotHeight;     // Snapshot height

	private:
		struct FSceneNode *rootFrame;     // Transient pointer to the rendered frame

	public:
		// XRootWindow interface
		void GetRootCursorPos(FLOAT *pPointX, FLOAT *pPointY);
		void SetRootCursorPos(FLOAT newPointX, FLOAT newPointY);

		XModalWindow *GetCurrentModal(void)
		{
			XWindow *pChild = GetTopChild();
			while (pChild)
			{
				if (pChild->windowType >= WIN_Modal)
					break;
				pChild = pChild->GetLowerSibling();
			}
			if (!pChild)
				pChild = this;
			return ((XModalWindow *)pChild);
		}

		APlayerPawnExt *GetRootPlayerPawn(void)  { return (parentPawn); }

		// Sound options
		void  EnablePositionalSound(UBOOL bEnabled=TRUE);
		UBOOL IsPositionalSoundEnabled(void)  { return (bPositionalSound); }

		// Rendering interface
		void  EnableRendering(UBOOL bRender=TRUE);
		UBOOL IsRenderingEnabled(void);
		void  SetRenderViewport(FLOAT newX, FLOAT newY, FLOAT newWidth, FLOAT newHeight);
		void  ResetRenderViewport(void);
		void  SetRawBackground(UTexture *texture=NULL, FColor newColor=FColor(255,255,255));
		void  SetRawBackgroundSize(FLOAT newWidth, FLOAT newHeight);
		void  StretchRawBackground(UBOOL bStretch=TRUE);
		UBOOL ConvertVectorToCoordinates(FVector location,
		                                 FLOAT *pRelativeX=NULL, FLOAT *pRelativeY=NULL);

		// Overall keyboard/mouse event handling
		void  GrabKeyboardEvents(void);
		void  UngrabKeyboardEvents(void);
		void  GrabMouseEvents(void);
		void  UngrabMouseEvents(void);
		UBOOL IsKeyboardGrabbed(void)     { return (handleKeyboardRef?TRUE:FALSE); }
		UBOOL IsMouseGrabbed(void)        { return (handleMouseRef?TRUE:FALSE);    }
		void  LockMouse(UBOOL bLockMove=TRUE, UBOOL bLockButton=TRUE);
		UBOOL IsMouseButton(EInputKey iKey);

		// Cursor routines
		void  ShowCursor(UBOOL bShow=TRUE);
		void  SetDefaultEditCursor(XCursor *newEditCursor=NULL);
		void  SetDefaultMovementCursors(XCursor *newMoveCursor=NULL,
		                                XCursor *newHorizontalCursor=NULL,
		                                XCursor *newVerticalCursor=NULL,
		                                XCursor *newTopLeftCursor=NULL,
		                                XCursor *newTopRightCursor=NULL);

		// Snapshot routines
		void SetSnapshotSize(FLOAT newWidth, FLOAT newHeight);
		UTexture *GenerateSnapshot(UTexture *useTexture = NULL, UBOOL bFilter = FALSE, UBOOL bColor = FALSE, UBOOL bCreateNewTexture = FALSE);

	protected:
		// XRootWindow interface callbacks
		virtual UBOOL HandleMouse(FLOAT newX, FLOAT newY);
		virtual UBOOL HandleVirtualKeyboard(EInputKey iKey, EInputAction state, UBOOL bRepeat);
		virtual UBOOL HandleKeyboard(TCHAR key);  // MEGA-BOOGER
		virtual UBOOL HandleButtons(EInputKey iKey, EInputAction state);

	public:
		// XWindow interface callbacks
		void  ConfigurationChanged(void);
		UBOOL ChildRequestedReconfiguration(XWindow *pChild);
		void  Tick(FLOAT deltaSeconds);

		// Routines invoked from XInputExt, XGameEngineExt, APlayerPawnExt, etc.
		void    PreRender(UCanvas *canvas);
		void    PostRender(UCanvas *canvas);
		void    PaintWindows(UCanvas *canvas);
		void    ResizeRoot(UCanvas *canvas);
		void    ProcessMouseMove(void);
		XWindow *GetMouseWindow(XModalWindow *pModal=NULL,
		                        FLOAT *pRelX=NULL, FLOAT *pRelY=NULL);

		UBOOL SetMouseDelta(FLOAT deltaX, FLOAT deltaY);
		UBOOL SetMousePosition(FLOAT newMouseX, FLOAT newMouseY);
		UBOOL Process(EInputKey iKey, EInputAction state, FLOAT delta);
		UBOOL Key(EInputKey iKey);

      void DescendantRemoved(XWindow *pDescendant);

	private:
		// Internal routines
		void InitializeWindows(void);
		void InvokeWindowReady(XWindow *pWindow);
		void RootTick(FLOAT deltaSeconds);
		void TickTree(XWindow *pWindow, FLOAT deltaSeconds);
		void ClampMousePosition(void);

		void ClipListInit(XClipRect &startClip, TArray<XClipRect> &clipList);
		void ClipListSubtract(XClipRect &pSub, TArray<XClipRect> &clipList);
		void ClipListGenerate(XWindow *pWindow, TArray<XClipRect> &clipList);
		void DrawRawBackground(XGC *gc, UTexture *texture, FColor rawColor);

	public:
		// Useful global root routines
		static void  DestroyAllWindows(void);
		static void  TickWindows(FLOAT deltaSeconds);
		static FLOAT GetWindowsTickOffset(UBOOL bReset=FALSE);

		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetDefaultEditCursor)
		DECLARE_FUNCTION(execSetDefaultMovementCursors)
		DECLARE_FUNCTION(execEnableRendering)
		DECLARE_FUNCTION(execIsRenderingEnabled)
		DECLARE_FUNCTION(execSetRenderViewport)
		DECLARE_FUNCTION(execResetRenderViewport)
		DECLARE_FUNCTION(execSetRawBackground)
		DECLARE_FUNCTION(execStretchRawBackground)
		DECLARE_FUNCTION(execSetRawBackgroundSize)
		DECLARE_FUNCTION(execEnablePositionalSound)
		DECLARE_FUNCTION(execIsPositionalSoundEnabled)
		DECLARE_FUNCTION(execLockMouse)
		DECLARE_FUNCTION(execShowCursor)
		DECLARE_FUNCTION(execSetSnapshotSize)
		DECLARE_FUNCTION(execGenerateSnapshot)

};  // XRootWindow


#endif // _EXT_ROOT_H_
