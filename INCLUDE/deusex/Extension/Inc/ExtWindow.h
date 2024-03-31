
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtWindow.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal window extension
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_WINDOW_H_
#define _EXT_WINDOW_H_


// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

class   UCanvas;
class   UTexture;

// Someday, we'll make this a real class
typedef UTexture XCursor;

struct  XWindowAttributes;

// Generic handle for a timer ID
typedef INT XTimerId;

// Generic timer data
typedef INT XTimerData;


// ----------------------------------------------------------------------
// EWinType - used as a quick way to distinguish window types

enum EWinType
{
	WIN_Normal   = 0,
	WIN_TabGroup = 1,
	WIN_Modal    = 2,
	WIN_Root     = 3
};


// ----------------------------------------------------------------------
// Structures
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// XTimerStruct structure - holds timer information (private)

struct XTimerStruct
{
	FLOAT        timeout;          // Duration of the timer
	FLOAT        timeRemaining;    // Time left before this is called
	FName        function;         // UnrealScript timer function to call
	UBOOL        bLoop;            // Repeat timer?
	XTimerData   clientData;       // User-specified data
	INT          refCount;         // 0 if this timer isn't being called
	UBOOL        bBeingDestroyed;  // TRUE if this timer is doomed
	XTimerStruct *nextTimer;       // Next chronological timer
};


// ----------------------------------------------------------------------
// NewWindow() - Macro to create a new window instance

#define NewWindow(UC, parent, bShow)  \
                 ((UC *)(XWindow::CreateNewWindow(UC::StaticClass, parent, bShow)))


// ----------------------------------------------------------------------
// XWindow class

class EXTENSION_API XWindow : public XExtensionObject
{

	DECLARE_CLASS(XWindow, XExtensionObject, 0)
	NO_DEFAULT_CONSTRUCTOR(XWindow)

	// There's some inbreeding here...
	friend XTabGroupWindow;
	friend XModalWindow;
	friend XRootWindow;

	public:
		XWindow(XWindow *parent);

		// Structors
		virtual void Init(XWindow *parent);
		virtual void CleanUp(void);
		void Destroy(void);

		// Quick class reference
		BYTE windowType;                // Contains window type

		XWindow *parentOwner;           // Window that "owns" this window

		// Booleans
		BITFIELD bIsVisible:1 GCC_PACK(4);  // TRUE if the window is visible
		BITFIELD bIsSensitive:1;        // TRUE if the window can take input
		BITFIELD bIsSelectable:1;       // TRUE if the window can have keyboard focus
		BITFIELD bIsInitialized:1;      // TRUE if the window has been initialized

		// Destructor information
		BITFIELD bBeingDestroyed:1;     // TRUE if this window is going bye-bye
		INT      lockCount GCC_PACK(4); // Reference count of locks on this window

		// Background drawing options
		BITFIELD bDrawRawBackground:1 GCC_PACK(4);  // TRUE if raw backgrounds should be drawn here
		BITFIELD bStretchBackground:1;              // TRUE=stretch background, FALSE=repeat tile
		BITFIELD bSmoothBackground:1;               // TRUE if the background should be smoothed
		BYTE     backgroundStyle GCC_PACK(4);       // Normal, masked or translucent

		// Text/font drawing state variables
		FColor    textColor;                    // Color of text
		FPlane    textPlane;                    // Plane of text; converted from textColor
		UFont     *normalFont;                  // Regular font
		UFont     *boldFont;                    // Boldface font
		FLOAT     textVSpacing;                 // Space between lines of text
		FColor    tileColor;                    // Color of tile
		FPlane    tilePlane;                    // Plane of tile; converted from tileColor
		BITFIELD  bSpecialText:1 GCC_PACK(4);   // TRUE if special text is enabled
		BITFIELD  bTextTranslucent:1;           // TRUE if translucent text is enabled

		// Configuration information
		FLOAT x GCC_PACK(4);         // X position of window relative to parent
		FLOAT y;                     // Y position of window relative to parent
		FLOAT width;                 // Width of window
		FLOAT height;                // Height of window

		// User-specified size preferences
		BYTE  winHAlign;             // User-specified horizontal alignment
		BYTE  winVAlign;             // User-specified vertical alignment
		FLOAT hMargin0;              // User-specified first horizontal margin
		FLOAT hMargin1;              // User-specified second horizontal margin (FULL only)
		FLOAT vMargin0;              // User-specified first vertical margin
		FLOAT vMargin1;              // User-specified second vertical margin (FULL only)
		FLOAT hardcodedWidth;        // User-specified preferred width
		FLOAT hardcodedHeight;       // User-specified preferred height

		// Temporary variables; used by parents to configure children
		FLOAT holdX;
		FLOAT holdY;
		FLOAT holdWidth;
		FLOAT holdHeight;

		// Hotkey assignment
		INT      acceleratorKey;                   // Hotkey, cast to an INT (hack)

		// Efficiency variables for geometry negotiation
		BITFIELD bNeedsReconfigure:1 GCC_PACK(4);  // TRUE if this window must be reconfigured
		BITFIELD bNeedsQuery:1;                    // TRUE if this window must be requeried
		BITFIELD bConfigured:1;                    // TRUE if this window was reconfigured
		BITFIELD bLastWidthSpecified:1;            // Whether width was specified in last size query
		BITFIELD bLastHeightSpecified:1;           // Whether height was specified in last size query
		FLOAT    lastSpecifiedWidth GCC_PACK(4);   // Width specified in last size query
		FLOAT    lastSpecifiedHeight;              // Height specified in last size query
		FLOAT    lastQueryWidth;                   // Width returned by last size query
		FLOAT    lastQueryHeight;                  // Height returned by last size query

		// Callback options
		BITFIELD bTickEnabled:1 GCC_PACK(4);       // TRUE if the UnrealScript Tick() is enabled

		// Multiple click options
		INT   maxClicks GCC_PACK(4);               // Number of clicks handled by window; 0=any number

		// Clipping rectangle
		XClipRect clipRect;          // Clipping rectangle; maintained at all times

		// Texture information
		UTexture *background;        // Background textures

		// Cursor information
		XCursor  *defaultCursor;       // Default cursor; can be overridden in CursorRequested
		UTexture *defaultCursorShadow; // Default cursor shadow; can be overridden in CursorRequested
		FLOAT    defaultHotX;          // Default hot X; can be overridden in CursorRequested
		FLOAT    defaultHotY;          // Default hot Y; can be overridden in CursorRequested
		FColor   defaultCursorColor;   // Default cursor color; can be overridden in CursorRequested

		// Sound stuff
		USound   *focusSound;        // Played when this window obtains focus
		USound   *unfocusSound;      // Played when this window loses focus
		USound   *visibleSound;      // Played when this window becomes visible
		USound   *invisibleSound;    // Played when this window becomes invisible
		FLOAT    soundVolume;	     // Volume of sound

		// Albert variables
		UObject *clientObject;       // Pointer to an object associated with this window

		// Traversal positions (used only when bIsSelectable is TRUE)
		INT     rowMajorIndex;       // Index into tab group's row-major sorted list
		INT     colMajorIndex;       // Index into tab group's column-major sorted list

		// Attribute pointer
		XWindowAttributes *firstAttribute;  // Pointer to first attribute; used by parent

		// Timer pointers
		XTimerStruct      *firstTimer;      // Pointer to this window's first timer
		XTimerStruct      *freeTimer;       // Free list of timers

		// Graphics contexts
		XGC *winGC;                  // This window's default graphics context
		XGC *getGC;                  // GC returned by GetGC() call

		// Relatives
		XWindow *parent;             // Parent window; NULL if this is root
		XWindow *firstChild;         // "Lowest" child (first one drawn)
		XWindow *lastChild;          // "Highest" child (last one drawn)
		XWindow *prevSibling;        // Next "lowest" sibling (previous one drawn)
		XWindow *nextSibling;        // Next "highest" sibling (next one drawn)

	public:
		// XWindow interface

		// Ancestral routines
		XRootWindow          *GetRootWindow(void);
		XModalWindow         *GetModalWindow(void);
		XTabGroupWindow      *GetTabGroupWindow(void);
		XWindow              *GetParent(void);
		class APlayerPawnExt *GetPlayerPawn(void);

		// Child routines
		XWindow         *GetBottomChild(UBOOL bVisibleOnly=TRUE);
		XWindow         *GetTopChild(UBOOL bVisibleOnly=TRUE);

		// Sibling routines
		XWindow         *GetLowerSibling(UBOOL bVisibleOnly=TRUE);
		XWindow         *GetHigherSibling(UBOOL bVisibleOnly=TRUE);

		// Destructor routines
		UBOOL           SafeDestroy(void);
		void            LockWindow(void);
		void            UnlockWindow(void);

		// InitWindow() - UnrealScript constructor for window
		void InitWindow(void)
		{
			ProcessScript(EXTENSION_InitWindow, NULL, TRUE);
		}

		// DestroyWindow() - UnrealScript destructor for window
		void DestroyWindow(void)
		{
			ProcessScript(EXTENSION_DestroyWindow, NULL, TRUE);
		}

		// WindowReady() - UnrealScript window initializer
		void WindowReady(void)
		{
			ProcessScript(EXTENSION_WindowReady, NULL, TRUE);
		}

		// Routines which change order of siblings
		void Raise(void);
		void Lower(void);

		// Visibility routines
		void  SetVisibility(UBOOL newVisibility);
		void  Show(void)  { SetVisibility(TRUE); }
		void  Hide(void)  { SetVisibility(FALSE); }
		UBOOL IsVisible(UBOOL bRecurse=TRUE)
		{
			if (bRecurse)
			{
				XWindow *pParent = this;
				while (pParent)
				{
					if (!pParent->bIsVisible)
						break;
					pParent = pParent->parent;
				}
				return (pParent ? FALSE : TRUE);
			}
			else
				return (bIsVisible);
		}

		// Sensitivity routines
		void  SetSensitivity(UBOOL newSensitivity);
		void  Enable(void)   { SetSensitivity(TRUE); }
		void  Disable(void)  { SetSensitivity(FALSE); }
		UBOOL IsSensitive(UBOOL bRecurse=TRUE)
		{
			if (bRecurse)
			{
				XWindow *pParent = this;
				while (pParent)
				{
					if (!pParent->bIsSensitive)
						break;
					pParent = pParent->parent;
				}
				return (pParent ? FALSE : TRUE);
			}
			else
				return (bIsSensitive);
		}

		// Selectability routines
		void  SetSelectability(UBOOL bIsSelectable);
		UBOOL IsSelectable(void)  { return (bIsSelectable); }
		UBOOL IsTraversable(UBOOL bCheckModal=TRUE);
		UBOOL IsFocusWindow(void);

		// Keypress query routine
		UBOOL IsKeyDown(int ikValue);

		// Get/Release calls for using the GC outside of drawing routines
		XGC  *GetGC(void);
		void ReleaseGC(XGC *gc);

		// Background texture calls
		void     SetBackground(UTexture *newBackground);
		UTexture *GetBackground(void)  { return (background); }

		// Background style calls
		void     SetBackgroundStyle(BYTE newStyle);
		BYTE     GetBackgroundStyle(void)  { return (backgroundStyle); }

		// Background stretching calls
		void     SetBackgroundStretching(UBOOL bIsStretched)  { bStretchBackground = bIsStretched; }
		UBOOL    GetBackgroundStretching(void)  { return (bStretchBackground); }

		// Background smoothing calls
		void     SetBackgroundSmoothing(UBOOL bIsSmoothed);
		UBOOL    GetBackgroundSmoothing(void)  { return (bSmoothBackground); }

		// Default font color calls
		void   SetTextColor(FColor newColor);
		FColor GetTextColor(void);
		void   EnableTranslucentText(UBOOL bNewTranslucency);
		UBOOL  IsTranslucentTextEnabled(void)  { return (bTextTranslucent); }

		// Routines to set the default font
		void SetFont(UFont *newFont);
		void SetNormalFont(UFont *newNormalFont);
		void SetBoldFont(UFont *newBoldFont);
		void SetFonts(UFont *newNormalFont, UFont *newBoldFont);
		void SetBaselineData(FLOAT newBaselineOffset=2, FLOAT newUnderlineHeight=1);

		// Tile color calls
		void   SetTileColor(FColor newColor);
		FColor GetTileColor(void);

		// Special text calls
		void   EnableSpecialText(UBOOL bEnabled);
		UBOOL  IsSpecialTextEnabled(void)  { return (bSpecialText); }

		// Reconfiguration options (set user preferences on x/y, width/height)
		void Resize(FLOAT newWidth, FLOAT newHeight);
		void Move(FLOAT newX, FLOAT newY);
		void Configure(FLOAT newX, FLOAT newY, FLOAT newWidth, FLOAT newHeight);
		void SetPos(FLOAT newX, FLOAT newY)
		{
			Move(newX, newY);
		}
		void SetSize(FLOAT newWidth, FLOAT newHeight)
		{
			Resize(newWidth, newHeight);
		}
		void SetConfiguration(FLOAT newX, FLOAT newY, FLOAT newWidth, FLOAT newHeight)
		{
			Configure(newX, newY, newWidth, newHeight);
		}
		void SetWidth(FLOAT newWidth);
		void SetHeight(FLOAT newHeight);
		void SetWindowAlignments(EHAlign hAlign, EVAlign vAlign,
		                         FLOAT hMargin0=0, FLOAT vMargin0=0,
		                         FLOAT hMargin1=0, FLOAT vMargin1=0);
		void SetHorizontalWindowAlignment(EHAlign hAlign, FLOAT hMargin0, FLOAT hMargin1);
		void SetVerticalWindowAlignment(EVAlign vAlign, FLOAT vMargin0, FLOAT vMargin1);
		void SetHorizontalWindowAlignment(EHAlign hAlign, FLOAT hMargin=0)
		{
			SetHorizontalWindowAlignment(hAlign, hMargin, hMargin);
		}
		void SetVerticalWindowAlignment(EVAlign vAlign, FLOAT vMargin=0)
		{
			SetVerticalWindowAlignment(vAlign, vMargin, vMargin);
		}

		// Configuration reset options (tells XWindow to ignore user preferences)
		void ResetWidth(void);
		void ResetHeight(void);
		void ResetSize(void);

		// Routines which associate an object with this window
		void    SetClientObject(UObject *newClientObject)  { clientObject = newClientObject; }
		UObject *GetClientObject(void)                     { return (clientObject); }

		// Routines which grab mouse input
		void  GrabMouse(void);
		void  UngrabMouse(void);

		// Routines to add and remove timers
		XTimerId AddTimer(FLOAT timeout, UBOOL bLoop=FALSE, XTimerData clientData=NULL,
		                  FName function=NAME_None);
		void     RemoveTimer(XTimerId timerId);

		// Routines to check actor status
		void  AddActorRef(class AActor *actor);
		void  RemoveActorRef(class AActor *actor);
		UBOOL IsActorValid(class AActor *actor);

		// Routines to get tick information
		FLOAT GetTickOffset(void);

		// Routine which sets/gets the current cursor position (relative to window)
		void  GetCursorPos(FLOAT *pPointX, FLOAT *pPointY);
		void  SetCursorPos(FLOAT newPointX, FLOAT newPointY);

		// Routine which sets the default cursor
		void  SetDefaultCursor(XCursor *newCursor, UTexture *newCursorShadow=NULL,
			                   FLOAT hotX=-1, FLOAT hotY=-1,
		                       FColor color=FColor(255,255,255));

		// Keyboard traversal routines
		XWindow *MoveFocus(EMove moveDir, UBOOL bForceWrap=FALSE);
		XWindow *MoveFocusLeft(void)          { return (MoveFocus(MOVE_Left));  }
		XWindow *MoveFocusRight(void)         { return (MoveFocus(MOVE_Right)); }
		XWindow *MoveFocusUp(void)            { return (MoveFocus(MOVE_Up));    }
		XWindow *MoveFocusDown(void)          { return (MoveFocus(MOVE_Down));  }

		// Tab group traversal routines
		XWindow *MoveTabGroup(EMove moveDir);
		XWindow *MoveTabGroupNext(void)       { return (MoveTabGroup(MOVE_Down)); }
		XWindow *MoveTabGroupPrev(void)       { return (MoveTabGroup(MOVE_Up));   }

		// Straight focus setting/getting routines
		UBOOL   SetFocusWindow(XWindow *pNewFocusWindow);
		XWindow *GetFocusWindow(void);

		// Hotkey routines
		void    SetAcceleratorText(const TCHAR *acceleratorText);
		void    SetAcceleratorKey(TCHAR key);
		TCHAR   GetAcceleratorKey(void)  { return ((TCHAR)acceleratorKey); }

		// Routines to find which window the specified point is in (for mouse traversal)
		XWindow *FindWindowByPoint(FLOAT pointX, FLOAT pointY,
		                           FLOAT *pRelativeX=NULL, FLOAT *pRelativeY=NULL);
		XWindow *FindWindow(FLOAT pointX, FLOAT pointY,
		                    FLOAT *pRelativeX=NULL, FLOAT *pRelativeY=NULL);

		// Routine which determines whether a point lies within a window (relative coordinates)
		UBOOL IsPointInWindow(FLOAT pointX, FLOAT pointY);

		// Routine that converts a viewport location into screen coordinates
		virtual UBOOL ConvertVectorToCoordinates(FVector location,
		                                         FLOAT *pRelativeX=NULL,
		                                         FLOAT *pRelativeY=NULL);

		// Sound routines
		void PlaySound(USound *sound, FLOAT volume, FLOAT pitch,
		               FLOAT posX, FLOAT posY);
		void PlaySound(USound *sound, FLOAT volume=-1.0, FLOAT pitch=1.0);
		void SetSoundVolume(FLOAT newVolume=1.0);

		void SetFocusSounds(USound *focusInSound=NULL, USound *focusOutSound=NULL);
		void SetVisibilitySounds(USound *showSound=NULL, USound *hideSound=NULL);

		// Slayer of innocent children
		void DestroyAllChildren(void)  { KillAllChildren(); }

		// Event invocations

		// Configuration negotiation routines...

		// Routine used by child which tells the parent to resize/move this window
		void AskParentForReconfigure(void);
		void InvalidateTreeConfiguration(void);
		void ReconfigureTree(UBOOL bInvalidateTree=TRUE);

		// Routines used by parent which ask a child how big it wants to be
		FLOAT QueryPreferredWidth(FLOAT queryHeight);
		FLOAT QueryPreferredHeight(FLOAT queryWidth);
		void  QueryPreferredSize(FLOAT *pPreferredWidth, FLOAT *pPreferredHeight);
		void  QueryPreferredSize(UBOOL bWidthSpecified, FLOAT queryWidth, FLOAT *pPreferredWidth,
		                         UBOOL bHeightSpecified, FLOAT queryHeight, FLOAT *pPreferredHeight);

		// Routine used by parent which asks a child what its granularity is
		void  QueryGranularity(FLOAT *pHGranularity, FLOAT *pVGranularity);

		// Routines used by parent to resize/show/hide child windows
		// (should NOT be used by anybody except parent!)
		void ResizeChild(void);
		void ConfigureChild(FLOAT newX, FLOAT newY, FLOAT newWidth, FLOAT newHeight);
		void SetChildVisibility(UBOOL bNewVisibility);

		// Routine used by child to ask its parent to make a part of it visible
		void AskParentToShowArea(FLOAT showX, FLOAT showY, FLOAT showWidth, FLOAT showHeight);

		// Returns cursor at cursor location
		XCursor *QueryCursor(FLOAT pointX, FLOAT pointY, FLOAT *pHotX=NULL, FLOAT *pHotY=NULL,
		                     FColor *pColor=NULL, UTexture **pCursorShadow=NULL);

		// Routine used to change styles
		void ChangeStyle(void);

		// Routine used to change the accelerator table
		void MakeAcceleratorTableDirty(void);

		// Routine that invokes UnrealScript events
		void ProcessScript(FName functionName, void *params=NULL, UBOOL bCheck=FALSE);


		// XWindow overrideable callbacks

		// Returns TRUE if mouse point is in window, or FALSE otherwise
		virtual UBOOL QueryPointInWindow(FLOAT pointX, FLOAT pointY)  { return (TRUE); }

		// Returns the current cursor
		virtual XCursor *CursorRequested(XWindow *win, FLOAT pointX, FLOAT pointY,
		                                 FLOAT &hotX, FLOAT &hotY, FColor &color,
										 UTexture **pCursorShadow)
		{
			struct { XWindow *win; FLOAT pointX; FLOAT pointY;
			         FLOAT hotX; FLOAT hotY; FColor color;
					 UTexture **cursorShadow;
			         XCursor *retval; } params;
			params.win          = win;
			params.pointX       = pointX;
			params.pointY       = pointY;
			params.hotX         = hotX;
			params.hotY         = hotY;
			params.color        = color;
			params.cursorShadow = pCursorShadow;
			params.retval = NULL;

			ProcessScript(EXTENSION_CursorRequested, &params);

			hotX  = params.hotX;
			hotY  = params.hotY;
			color = params.color;
			pCursorShadow = params.cursorShadow;

			return (params.retval);
		}

		// Called when a child window is added or removed
		virtual void ChildAdded(XWindow *pChild)
		{
			struct { XWindow *pChild; } params;
			params.pChild = pChild;
			ProcessScript(EXTENSION_ChildAdded, &params);
		}
		virtual void ChildRemoved(XWindow *pChild)
		{
			struct { XWindow *pChild; } params;
			params.pChild = pChild;
			ProcessScript(EXTENSION_ChildRemoved, &params);
		}
		virtual void DescendantAdded(XWindow *pDescendant)
		{
			struct { XWindow *pDescendant; } params;
			params.pDescendant = pDescendant;
			ProcessScript(EXTENSION_DescendantAdded, &params);
		}
		virtual void DescendantRemoved(XWindow *pDescendant)
		{
			struct { XWindow *pDescendant; } params;
			params.pDescendant = pDescendant;
			ProcessScript(EXTENSION_DescendantRemoved, &params);
		}

		// Called when a C++ timer is invoked
		virtual void Timer(XTimerId timerId, INT invocations, XTimerData clientData);

		// Called when the parent wants to know this window's preferred size
		virtual void ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                          UBOOL bHeightSpecified, FLOAT &preferredHeight);

		// Called when the parent wants to know the granularity of this window
		virtual void ParentRequestedGranularity(FLOAT &hGranulatity, FLOAT &vGranularity);

		// Called when a child invokes Show() or Hide()
		virtual void ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility)
		{
			struct { XWindow *pChild; BITFIELD bNewVisibility:1; } params;
			params.pChild         = pChild;
			params.bNewVisibility = bNewVisibility;
			ProcessScript(EXTENSION_ChildRequestedVisibilityChange, &params);
		}

		// Called when a child invokes AskParentForReconfigure()
		virtual UBOOL ChildRequestedReconfiguration(XWindow *pChild)
		{
			struct { XWindow *pChild; BITFIELD retval; } params;
			params.pChild = pChild;
			params.retval = FALSE;
			ProcessScript(EXTENSION_ChildRequestedReconfiguration, &params);
			return (params.retval);
		}

		virtual void ChildRequestedShowArea(XWindow *pChild, FLOAT showX, FLOAT showY,
		                                    FLOAT showWidth, FLOAT showHeight)
		{
			struct { XWindow *pChild; FLOAT showX; FLOAT showY;
			         FLOAT showWidth; FLOAT showHeight; } params;
			params.pChild     = pChild;
			params.showX      = showX;
			params.showY      = showY;
			params.showWidth  = showWidth;
			params.showHeight = showHeight;
			ProcessScript(EXTENSION_ChildRequestedShowArea, &params);
		}

		// Called when the parent changes this window's size
		// (guaranteed to be called during AskParentForReconfigure(), unless
		// child is invisible)

		virtual void  ConfigurationChanged(void)
		{
			ProcessScript(EXTENSION_ConfigurationChanged, NULL);
		}

		// Called when the parent shows or hides this window
		virtual void  VisibilityChanged(UBOOL bNewVisibility)
		{
			struct { BITFIELD bNewVisibility; } params;
			params.bNewVisibility = bNewVisibility;
			ProcessScript(EXTENSION_VisibilityChanged, &params);
		}

		// Called when a window becomes sensitive or insensitive
		virtual void  SensitivityChanged(UBOOL bNewSensitivity)
		{
			struct { BITFIELD bNewSensitivity; } params;
			params.bNewSensitivity = bNewSensitivity;
			ProcessScript(EXTENSION_SensitivityChanged, &params);
		}

		// Called when the mouse moves within this window
		virtual void  MouseMoved(FLOAT pointX, FLOAT pointY)
		{
			struct { FLOAT pointX; FLOAT pointY; } params;
			params.pointX = pointX;
			params.pointY = pointY;
			ProcessScript(EXTENSION_MouseMoved, &params);
		}

		// Called when the mouse enters this window
		virtual void  MouseEnteredWindow(void)
		{
			ProcessScript(EXTENSION_MouseEnteredWindow, NULL);
		}

		// Called when the mouse leaves this window
		virtual void  MouseLeftWindow(void)
		{
			ProcessScript(EXTENSION_MouseLeftWindow, NULL);
		}

		// Called when keyboard input focus is going to this window
		virtual void  FocusEnteredWindow(void)
		{
			ProcessScript(EXTENSION_FocusEnteredWindow, NULL);
		}

		// Called when keyboard input focus is no longer going to this window
		virtual void  FocusLeftWindow(void)
		{
			ProcessScript(EXTENSION_FocusLeftWindow, NULL);
		}

		// Called when keyboard input focus is going to a descendant
		virtual void FocusEnteredDescendant(XWindow *enterWindow)
		{
			struct { XWindow *enterWindow; } params;
			params.enterWindow = enterWindow;
			ProcessScript(EXTENSION_FocusEnteredDescendant, &params);
		}

		// Called when keyboard input focus is no longer going to a descendant
		virtual void FocusLeftDescendant(XWindow *leaveWindow)
		{
			struct { XWindow *leaveWindow; } params;
			params.leaveWindow = leaveWindow;
			ProcessScript(EXTENSION_FocusLeftDescendant, &params);
		}

		// Raw mouse button press callback; returns TRUE if handled
		virtual UBOOL RawMouseButtonPressed(FLOAT pointX, FLOAT pointY,
		                                    EInputKey button, EInputAction state)
		{
			struct { FLOAT pointX; FLOAT pointY; BYTE button; BYTE state; BITFIELD retval; } params;
			params.pointX = pointX;
			params.pointY = pointY;
			params.button = (BYTE)(button);
			params.state  = (BYTE)state;
			params.retval = FALSE;
			ProcessScript(EXTENSION_RawMouseButtonPressed, &params);
			return (params.retval);
		}

		// Raw keyboard input callback; returns TRUE if handled
		virtual UBOOL RawKeyPressed(EInputKey key, EInputAction state,
		                            UBOOL bRepeat)
		{
			struct { BYTE key; BYTE state; BITFIELD bRepeat; BITFIELD retval; } params;
			params.key     = (BYTE)(key);
			params.state   = (BYTE)state;
			params.bRepeat = bRepeat;
			params.retval  = FALSE;
			ProcessScript(EXTENSION_RawKeyPressed, &params);
			return (params.retval);
		}

		// Called when a mouse button is pressed (if raw routine didn't handle)
		virtual UBOOL MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                                 INT numClicks)
		{
			struct { FLOAT pointX; FLOAT pointY; BYTE button; INT numClicks;
			         BITFIELD retval; } params;
			params.pointX    = pointX;
			params.pointY    = pointY;
			params.button    = (BYTE)(button);
			params.numClicks = numClicks;
			params.retval    = FALSE;
			ProcessScript(EXTENSION_MouseButtonPressed, &params);
			return (params.retval);
		}

		// Called when a mouse button is released (if raw routine didn't handle)
		virtual UBOOL MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                                  INT numClicks)
		{
			struct { FLOAT pointX; FLOAT pointY; BYTE button; INT numClicks;
			         BITFIELD retval; } params;
			params.pointX    = pointX;
			params.pointY    = pointY;
			params.button    = (BYTE)(button);
			params.numClicks = numClicks;
			params.retval    = FALSE;
			ProcessScript(EXTENSION_MouseButtonReleased, &params);
			return (params.retval);
		}

		// Called when a key has been pressed; uses ASCII codes
		virtual UBOOL KeyPressed(TCHAR key)
		{
			TCHAR keyStr[2];
			keyStr[0] = key;
			keyStr[1] = '\0';
			struct { FString key; BITFIELD retval; } params;
			params.key    = keyStr;
			params.retval = FALSE;
			ProcessScript(EXTENSION_KeyPressed, &params);
			return (params.retval);
		}

		// Called when a key has been pressed; uses virtual key codes (IK_)
		virtual UBOOL VirtualKeyPressed(EInputKey key, UBOOL bRepeat)
		{
			struct { BYTE key; BITFIELD bRepeat; BITFIELD retval; } params;
			params.key     = (BYTE)(key);
			params.bRepeat = bRepeat;
			params.retval  = FALSE;
			ProcessScript(EXTENSION_VirtualKeyPressed, &params);
			return (params.retval);
		}

		// Called when this window's accelerator key is pressed
		virtual UBOOL AcceleratorKeyPressed(TCHAR key)
		{
			TCHAR keyStr[2];
			keyStr[0] = key;
			keyStr[1] = '\0';
			struct { FString key; BITFIELD retval; } params;
			params.key    = keyStr;
			params.retval = FALSE;
			ProcessScript(EXTENSION_AcceleratorKeyPressed, &params);
			return (params.retval);
		}

		// Called periodically
		virtual void Tick(FLOAT deltaSeconds)  {}

		// Window drawing routine
		virtual void Draw(XGC *gc)
		{
			guard(XWindow::Draw);
			struct {class XGC *gc;} parms;
			parms.gc = gc;
			ProcessScript(EXTENSION_DrawWindow, &parms);
			unguard;
		}

		// Post window drawing routine; called after all children are drawn
		virtual void PostDraw(XGC *gc)
		{
			guard(XWindow::PostDraw);
			struct {class XGC *gc;} parms;
			parms.gc = gc;
			ProcessScript(EXTENSION_PostDrawWindow, &parms);
			unguard;
		}

		// Called when another window grabs the mouse from this one
		virtual void  MouseGrabLost(XWindow *newWindow)  {}

		// Called when the window style is changed
		virtual void  StyleChanged(void)
		{
			ProcessScript(EXTENSION_StyleChanged, NULL, FALSE);
		}

		// Called when a ButtonActivated event is invoked
		virtual UBOOL ButtonActivated(XWindow *pButton)
		{
			struct { XWindow *pButton; BITFIELD retval; } params;
			params.pButton = pButton;
			params.retval  = FALSE;
			ProcessScript(EXTENSION_ButtonActivated, &params);
			return (params.retval);
		}

		// Called when a ButtonActivated event is invoked for
		// the RIGHT mouse button
		virtual UBOOL ButtonActivatedRight(XWindow *pButton)
		{
			struct { XWindow *pButton; BITFIELD retval; } params;
			params.pButton = pButton;
			params.retval  = FALSE;
			ProcessScript(EXTENSION_ButtonActivatedRight, &params);
			return (params.retval);
		}

		// Called when a ToggleChanged event is invoked
		virtual UBOOL ToggleChanged(XWindow *pButton, UBOOL bToggleValue)
		{
			struct { XWindow *pButton; BITFIELD bToggleValue; BITFIELD retval; } params;
			params.pButton      = pButton;
			params.bToggleValue = bToggleValue;
			params.retval       = FALSE;
			ProcessScript(EXTENSION_ToggleChanged, &params);
			return (params.retval);
		}

		// Called when a BoxOptionSelected event is invoked
		virtual UBOOL BoxOptionSelected(XWindow *pBox, INT buttonNumber)
		{
			struct { XWindow *pBox; INT buttonNumber; BITFIELD retval; } params;
			params.pBox         = pBox;
			params.buttonNumber = buttonNumber;
			params.retval       = FALSE;
			ProcessScript(EXTENSION_BoxOptionSelected, &params);
			return (params.retval);
		}

		// Called when a ScalePositionChanged event is invoked
		virtual UBOOL ScalePositionChanged(XWindow *pScale, INT newTickPosition,
		                                   FLOAT newValue, UBOOL bFinal)
		{
			struct { XWindow *pScale; INT newTickPosition; FLOAT newValue;
			         BITFIELD bFinal; BITFIELD retval; } params;
			params.pScale          = pScale;
			params.newTickPosition = newTickPosition;
			params.newValue        = newValue;
			params.bFinal          = bFinal;
			params.retval          = FALSE;
			ProcessScript(EXTENSION_ScalePositionChanged, &params);
			return (params.retval);
		}

		// Called when a ScaleRangeChanged event is invoked
		virtual UBOOL ScaleRangeChanged(XWindow *pScale,
		                                INT newFromTick, INT newToTick,
		                                FLOAT newFromValue, FLOAT newToValue,
		                                UBOOL bFinal)
		{
			struct { XWindow *pScale; INT newFromTick; INT newToTick;
			         FLOAT newFromValue; FLOAT newToValue; BITFIELD bFinal;
			         BITFIELD retval; } params;
			params.pScale          = pScale;
			params.newFromTick     = newFromTick;
			params.newToTick       = newToTick;
			params.newFromValue    = newFromValue;
			params.newToValue      = newToValue;
			params.bFinal          = bFinal;
			params.retval          = FALSE;
			ProcessScript(EXTENSION_ScaleRangeChanged, &params);
			return (params.retval);
		}

		// Called when a ScaleAttributesChanged event is invoked
		virtual UBOOL ScaleAttributesChanged(XWindow *pScale,
		                                     INT tickPosition, INT tickSpan,
		                                     INT numTicks)
		{
			struct { XWindow *pScale; INT tickPosition; INT tickSpan;
			         INT numTicks; BITFIELD retval; } params;
			params.pScale          = pScale;
			params.tickPosition    = tickPosition;
			params.tickSpan        = tickSpan;
			params.numTicks        = numTicks;
			params.retval          = FALSE;
			ProcessScript(EXTENSION_ScaleAttributesChanged, &params);
			return (params.retval);
		}

		// Called when a ClipAttributesChanged event is invoked
		virtual UBOOL ClipAttributesChanged(XWindow *pClip,
		                                    INT newClipWidth, INT newClipHeight,
		                                    INT newChildWidth, INT newChildHeight)
		{
			struct { XWindow *pClip; INT newClipWidth; INT newClipHeight;
			         INT newChildWidth; INT newChildHeight; BITFIELD retval; } params;
			params.pClip          = pClip;
			params.newClipWidth   = newClipWidth;
			params.newClipHeight  = newClipHeight;
			params.newChildWidth  = newChildWidth;
			params.newChildHeight = newChildHeight;
			params.retval         = FALSE;
			ProcessScript(EXTENSION_ClipAttributesChanged, &params);
			return (params.retval);
		}

		// Called when a ListRowActivated event is invoked
		virtual UBOOL ListRowActivated(XWindow *pList, INT rowId)
		{
			struct { XWindow *pList; INT rowId;
			         BITFIELD retval; } params;
			params.pList  = pList;
			params.rowId  = rowId;
			params.retval = FALSE;
			ProcessScript(EXTENSION_ListRowActivated, &params);
			return (params.retval);
		}

		// Called when a ListSelectionChanged event is invoked
		virtual UBOOL ListSelectionChanged(XWindow *pList, INT numSelections, INT focusRowId)
		{
			struct { XWindow *pList; INT numSelections; INT focusRowId;
			         BITFIELD retval; } params;
			params.pList         = pList;
			params.focusRowId    = focusRowId;
			params.numSelections = numSelections;
			params.retval        = FALSE;
			ProcessScript(EXTENSION_ListSelectionChanged, &params);
			return (params.retval);
		}

		// Called when a ClipPositionChanged event is invoked
		virtual UBOOL ClipPositionChanged(XWindow *pClip, INT newCol, INT newRow)
		{
			struct { XWindow *pClip; INT newCol; INT newRow;
			         BITFIELD retval; } params;
			params.pClip  = pClip;
			params.newCol = newCol;
			params.newRow = newRow;
			params.retval = FALSE;
			ProcessScript(EXTENSION_ClipPositionChanged, &params);
			return (params.retval);
		}

		// Called when a TextChanged event is invoked
		virtual UBOOL TextChanged(XWindow *pEdit, UBOOL bModified)
		{
			struct { XWindow *pEdit; BITFIELD bModified; BITFIELD retval; } params;
			params.pEdit     = pEdit;
			params.bModified = bModified;
			params.retval    = FALSE;
			ProcessScript(EXTENSION_TextChanged, &params);
			return (params.retval);
		}

		// Called when a EditActivated event is invoked
		virtual UBOOL EditActivated(XWindow *pEdit, UBOOL bModified)
		{
			struct { XWindow *pEdit; BITFIELD bModified; BITFIELD retval; } params;
			params.pEdit     = pEdit;
			params.bModified = bModified;
			params.retval    = FALSE;
			ProcessScript(EXTENSION_EditActivated, &params);
			return (params.retval);
		}

		// Coordinate conversion routines; converts between two windows
		static void ConvertCoordinates(XWindow *fromWin, FLOAT fromX, FLOAT fromY,
		                               XWindow *toWin,   FLOAT *pToX=NULL, FLOAT *pToY=NULL);

		// Creates a new window of the specified type
		static XWindow *CreateNewWindow(UClass *newClass, XWindow *parentWindow, UBOOL bShow=TRUE);

	protected:
		// Converts special UnrealScript characters to normal characters
		const TCHAR *ConvertScriptString(const TCHAR *oldStr);

		// Should be called before anything else in all intrinsic Destroy() methods
		void PreDestroy(void);

		// Anybody who kills children can't be all bad...
		void KillAllChildren(void);

		// Window attribute routines (used by parent)
		XWindowAttributes *GetWindowAttributes(UClass *parentClass);
		void              DeleteWindowAttributes(UClass *parentClass);

	private:
		// Child adopter/orphaner routines
		void AddChild(XWindow *child, UBOOL bRaise=TRUE);
		void RemoveChild(XWindow *child);

		// Insures that the focus window can still have focus
		void CheckFocusWindow(void);
		void CheckGrabbedWindow(void);

		// Handles RootWindow's initCount variable
		void RecomputeInitTree(UBOOL bNewVisibility);

		// Calls VisibilityChanged() for the windows that need it
		void InvokeVisibilityChange(UBOOL bNewVisibility);

		// Calls SensitivityChanged() for the windows that need it
		void InvokeSensitivityChange(UBOOL bNewSensitivity);

		// Calls ConfigurationChanged() for the current window
		void ChangeConfiguration(void);

		// Calls StyleChanged() for the window hierarchy
		void ChangeStyleTree(void);

		// Invokes all timers for this window
		void InvokeTimers(FLOAT deltaSeconds);

		// Sets clipping on an entire window tree
		void ClipTree(void);

		// Draws all windows
		void DrawTree(UCanvas *canvas, UTexture *debugTexture, FLOAT frameTimer,
		              INT hMult, INT vMult);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execDestroy)
		DECLARE_FUNCTION(execNewChild)
		DECLARE_FUNCTION(execRaise)
		DECLARE_FUNCTION(execLower)
		DECLARE_FUNCTION(execShow)
		DECLARE_FUNCTION(execHide)
		DECLARE_FUNCTION(execIsVisible)
		DECLARE_FUNCTION(execSetSensitivity)
		DECLARE_FUNCTION(execEnable)
		DECLARE_FUNCTION(execDisable)
		DECLARE_FUNCTION(execIsSensitive)
		DECLARE_FUNCTION(execSetSelectability)
		DECLARE_FUNCTION(execSetBackground)
		DECLARE_FUNCTION(execSetBackgroundStyle)
		DECLARE_FUNCTION(execSetBackgroundSmoothing)
		DECLARE_FUNCTION(execSetBackgroundStretching)

		DECLARE_FUNCTION(execGetRootWindow)
		DECLARE_FUNCTION(execGetModalWindow)
		DECLARE_FUNCTION(execGetTabGroupWindow)
		DECLARE_FUNCTION(execGetParent)
		DECLARE_FUNCTION(execGetPlayerPawn)

		DECLARE_FUNCTION(execSetConfiguration)
		DECLARE_FUNCTION(execSetSize)
		DECLARE_FUNCTION(execSetPos)
		DECLARE_FUNCTION(execSetWidth)
		DECLARE_FUNCTION(execSetHeight)
		DECLARE_FUNCTION(execResetSize)
		DECLARE_FUNCTION(execResetWidth)
		DECLARE_FUNCTION(execResetHeight)
		DECLARE_FUNCTION(execSetWindowAlignments)

		DECLARE_FUNCTION(execSetAcceleratorText)

		DECLARE_FUNCTION(execSetFocusWindow)
		DECLARE_FUNCTION(execGetFocusWindow)
		DECLARE_FUNCTION(execMoveFocusLeft)
		DECLARE_FUNCTION(execMoveFocusRight)
		DECLARE_FUNCTION(execMoveFocusUp)
		DECLARE_FUNCTION(execMoveFocusDown)
		DECLARE_FUNCTION(execMoveTabGroupNext)
		DECLARE_FUNCTION(execMoveTabGroupPrev)
		DECLARE_FUNCTION(execIsFocusWindow)

		DECLARE_FUNCTION(execConvertCoordinates)

		DECLARE_FUNCTION(execGrabMouse)
		DECLARE_FUNCTION(execUngrabMouse)
		DECLARE_FUNCTION(execGetCursorPos)
		DECLARE_FUNCTION(execSetCursorPos)
		DECLARE_FUNCTION(execSetDefaultCursor)

		DECLARE_FUNCTION(execGetTopChild)
		DECLARE_FUNCTION(execGetBottomChild)
		DECLARE_FUNCTION(execGetHigherSibling)
		DECLARE_FUNCTION(execGetLowerSibling)

		DECLARE_FUNCTION(execDestroyAllChildren)

		DECLARE_FUNCTION(execAskParentForReconfigure)
		DECLARE_FUNCTION(execConfigureChild)
		DECLARE_FUNCTION(execResizeChild)
		DECLARE_FUNCTION(execQueryPreferredWidth)
		DECLARE_FUNCTION(execQueryPreferredHeight)
		DECLARE_FUNCTION(execQueryPreferredSize)
		DECLARE_FUNCTION(execQueryGranularity)
		DECLARE_FUNCTION(execSetChildVisibility)

		DECLARE_FUNCTION(execAskParentToShowArea)

		DECLARE_FUNCTION(execConvertScriptString)

		DECLARE_FUNCTION(execIsKeyDown)
		DECLARE_FUNCTION(execIsPointInWindow)
		DECLARE_FUNCTION(execFindWindow)

		DECLARE_FUNCTION(execPlaySound)
		DECLARE_FUNCTION(execSetSoundVolume)

		DECLARE_FUNCTION(execSetTileColor)
		DECLARE_FUNCTION(execSetTextColor)
		DECLARE_FUNCTION(execSetFont)
		DECLARE_FUNCTION(execSetFonts)
		DECLARE_FUNCTION(execSetNormalFont)
		DECLARE_FUNCTION(execSetBoldFont)
		DECLARE_FUNCTION(execEnableSpecialText)
		DECLARE_FUNCTION(execEnableTranslucentText)

		DECLARE_FUNCTION(execSetBaselineData)

		DECLARE_FUNCTION(execCarriageReturn)

		DECLARE_FUNCTION(execGetGC)
		DECLARE_FUNCTION(execReleaseGC)

		DECLARE_FUNCTION(execSetClientObject)
		DECLARE_FUNCTION(execGetClientObject)

		DECLARE_FUNCTION(execConvertVectorToCoordinates)

		DECLARE_FUNCTION(execAddTimer)
		DECLARE_FUNCTION(execRemoveTimer)
		DECLARE_FUNCTION(execGetTickOffset)

		DECLARE_FUNCTION(execChangeStyle)

		DECLARE_FUNCTION(execSetFocusSounds)
		DECLARE_FUNCTION(execSetVisibilitySounds)

		DECLARE_FUNCTION(execAddActorRef)
		DECLARE_FUNCTION(execRemoveActorRef)
		DECLARE_FUNCTION(execIsActorValid)

};  // XWindow


// ----------------------------------------------------------------------
// XWindowAttributes - parent-specific information stored for each child

struct XWindowAttributes
{

	// Constructor
	XWindowAttributes(XWindow *newWinChild, UClass *newParentClass)
	{
		parentClass = newParentClass;
		winChild    = newWinChild;

		// Add this attribute to the linked list of attributes
		if (winChild)
		{
			nextAttribute = winChild->firstAttribute;
			winChild->firstAttribute = this;
		}
	}

	// Destructor
	virtual ~XWindowAttributes()
	{
		if (winChild)
		{
			XWindowAttributes *pCurAtt, *pPrevAtt;
			pCurAtt  = winChild->firstAttribute;
			pPrevAtt = NULL;

			// Find this attribute in the linked list of attributes
			while (pCurAtt)
			{
				if (pCurAtt == this)
					break;
				pPrevAtt = pCurAtt;
				pCurAtt  = pCurAtt->nextAttribute;
			}

			// Remove it
			if (pCurAtt)
			{
				if (pPrevAtt)
					pPrevAtt->nextAttribute = nextAttribute;
				else
					winChild->firstAttribute = nextAttribute;
			}
			nextAttribute = NULL;
		}
	}

	// Members
	UClass            *parentClass;
	XWindow           *winChild;
	XWindowAttributes *nextAttribute;

};  // XWindowAttributes


#endif // _EXT_WINDOW_H_
