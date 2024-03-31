
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtScrollArea.h
//  Programmer  :  Scott Martin
//  Description :  Header file for scrolling area window class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_SCROLL_AREA_H_
#define _EXT_SCROLL_AREA_H_


// ----------------------------------------------------------------------
// XScrollAreaWindow class

class EXTENSION_API XScrollAreaWindow : public XWindow
{
	DECLARE_CLASS(XScrollAreaWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XScrollAreaWindow)

	public:
		XScrollAreaWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

	protected:
		XScaleManagerWindow *hScaleMgr;
		XScaleManagerWindow *vScaleMgr;
		XScaleWindow        *hScale;
		XScaleWindow        *vScale;
		XButtonWindow       *leftButton;
		XButtonWindow       *rightButton;
		XButtonWindow       *upButton;
		XButtonWindow       *downButton;
		XClipWindow         *clipWindow;

		FLOAT               marginWidth;
		FLOAT               marginHeight;

		FLOAT               scrollbarDistance;

		BITFIELD            bHideScrollbars:1 GCC_PACK(4);
		BITFIELD            bHLastShow:1;
		BITFIELD            bVLastShow:1;

	public:
		// XScrollAreaWindow interface
		void EnableScrolling(UBOOL bHScrolling=TRUE, UBOOL bVScrolling=TRUE);

		void SetScrollbarDistance(FLOAT newDistance)
		{
			if (scrollbarDistance != newDistance)
			{
				scrollbarDistance = newDistance;
				AskParentForReconfigure();
			}
		}

		void SetMargins(FLOAT newMarginWidth, FLOAT newMarginHeight)
		{
			if ((marginWidth != newMarginWidth) || (marginHeight != newMarginHeight))
			{
				marginWidth  = newMarginWidth;
				marginHeight = newMarginHeight;
				AskParentForReconfigure();
			}
		}

		void AutoHideScrollbars(UBOOL bAutoHide=TRUE)
		{
			if ((UBOOL)bHideScrollbars != bAutoHide)
			{
				bHideScrollbars = bAutoHide;
				AskParentForReconfigure();
			}
		}

		void GetButtons(XButtonWindow **pUpButton=NULL, XButtonWindow **pDownButton=NULL,
		                XButtonWindow **pLeftButton=NULL, XButtonWindow **pRightButton=NULL);
		void GetScrollbars(XScaleWindow **pHScale=NULL, XScaleWindow **pVScale=NULL);
		void GetScrollManagers(XScaleManagerWindow **pHMgr=NULL, XScaleManagerWindow **pVMgr=NULL);
		XClipWindow *GetClipWindow(void)  { return (clipWindow); }

		// XWindow interface callbacks
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		UBOOL   ChildRequestedReconfiguration(XWindow *pChild);
		void    ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		void    ConfigurationChanged(void);

		void    DescendantRemoved(XWindow *pChild);

		UBOOL   ScaleRangeChanged(XWindow *pScale,
		                          INT newFromTick, INT newToTick,
		                          FLOAT newFromValue, FLOAT newToValue, UBOOL bFinal);

		UBOOL   ClipAttributesChanged(XWindow *pClip,
		                              INT newClipWidth, INT newClipHeight,
		                              INT newChildWidth, INT newChildHeight);

		UBOOL   ClipPositionChanged(XWindow *pClip, INT newCol, INT newRow);
		UBOOL MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                         INT numClicks);

	private:
		// Internal routines used by this class
		void ComputeChildSizes(UBOOL bWidthSpecified,
		                       FLOAT queryWidth,
		                       FLOAT *pTotalWidth,
		                       UBOOL bHeightSpecified,
		                       FLOAT queryHeight,
		                       FLOAT *pTotalHeight);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execEnableScrolling)
		DECLARE_FUNCTION(execSetScrollbarDistance)
		DECLARE_FUNCTION(execSetAreaMargins)
		DECLARE_FUNCTION(execAutoHideScrollbars)

};  // XScrollAreaWindow


#endif // _EXT_SCROLL_AREA_H_

