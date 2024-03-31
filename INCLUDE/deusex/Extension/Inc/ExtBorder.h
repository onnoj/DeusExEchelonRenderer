
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtBorder.h
//  Programmer  :  Scott Martin
//  Description :  Header file for border window class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_BORDER_H_
#define _EXT_BORDER_H_


// ----------------------------------------------------------------------
// XBorderWindow class

class EXTENSION_API XBorderWindow : public XWindow
{
	DECLARE_CLASS(XBorderWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XBorderWindow)

	public:
		XBorderWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

	protected:
		// Border textures
		UTexture *borderLeft;
		UTexture *borderTopLeft;
		UTexture *borderTop;
		UTexture *borderTopRight;
		UTexture *borderRight;
		UTexture *borderBottomRight;
		UTexture *borderBottom;
		UTexture *borderBottomLeft;

		UTexture *center;

		// Cursors
		XCursor  *moveCursor;
		XCursor  *hMoveCursor;
		XCursor  *vMoveCursor;
		XCursor  *tlMoveCursor;
		XCursor  *trMoveCursor;

		// Drawing options
		BYTE     borderStyle;                  // Solid, translucent or masked
		BITFIELD bSmoothBorder:1 GCC_PACK(4);  // TRUE if the borders should be smoothed
		BITFIELD bStretchBorder:1;             // TRUE=stretched, FALSE=repeated tile

		// Window options
		BITFIELD bResizeable:1;                // TRUE if the user can resize by dragging
		BITFIELD bMarginsFromBorder:1;         // TRUE if child margins are based on borders

		// Margins for child windows
		FLOAT    childLeftMargin GCC_PACK(4);
		FLOAT    childRightMargin;
		FLOAT    childTopMargin;
		FLOAT    childBottomMargin;

	private:
		// Amount of space required by border textures
		FLOAT    leftMargin;
		FLOAT    rightMargin;
		FLOAT    topMargin;
		FLOAT    bottomMargin;

		// TRUE if the window is being dragged
		BITFIELD bLeftDrag:1 GCC_PACK(4);
		BITFIELD bRightDrag:1;
		BITFIELD bUpDrag:1;
		BITFIELD bDownDrag:1;

		// Coordinates for dragging
		FLOAT    lastMouseX GCC_PACK(4);
		FLOAT    lastMouseY;
		FLOAT    dragX;
		FLOAT    dragY;
		FLOAT    dragWidth;
		FLOAT    dragHeight;

	public:
		// XBorderWindow interface
		void SetBorders(UTexture *bordTL, UTexture *bordTR,
		                UTexture *bordBL, UTexture *bordBR,
		                UTexture *bordL,  UTexture *bordR,
		                UTexture *bordT,  UTexture *bordB,
		                UTexture *center=NULL);
		void SetBorderMargins(FLOAT newLeft=0, FLOAT newRight=0,
		                      FLOAT newTop=0, FLOAT newBottom=0);
		void BaseMarginsFromBorder(UBOOL bBorder=TRUE);

		void EnableResizing(UBOOL bNewResize=TRUE);

		void SetMoveCursors(XCursor *newMove=NULL,
		                    XCursor *newHMove=NULL,
		                    XCursor *newVMove=NULL,
		                    XCursor *newTLMove=NULL,
		                    XCursor *newTRMove=NULL);

		// XWindow interface callbacks
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		UBOOL   ChildRequestedReconfiguration(XWindow *pChild);
		void    ConfigurationChanged(void);
		XCursor *CursorRequested(XWindow *win, FLOAT pointX, FLOAT pointY,
		                         FLOAT &hotX, FLOAT &hotY, FColor &newColor, 
								 UTexture **pCursorShadow);
		void    Draw(XGC *gc);

		UBOOL   MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                           INT numClicks);
		UBOOL   MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                            INT numClicks);
		void    MouseMoved(FLOAT pointX, FLOAT pointY);

	private:
		// Internal routines used by this class
		void ComputeWindowBorders(void);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetBorders)
		DECLARE_FUNCTION(execSetBorderMargins)
		DECLARE_FUNCTION(execBaseMarginsFromBorder)
		DECLARE_FUNCTION(execEnableResizing)
		DECLARE_FUNCTION(execSetMoveCursors)

};  // XBorderWindow


#endif // _EXT_BORDER_H_
