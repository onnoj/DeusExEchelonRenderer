
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtClipWindow.h
//  Programmer  :  Scott Martin
//  Description :  Header file for clipping window class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_CLIP_WINDOW_H_
#define _EXT_CLIP_WINDOW_H_


// ----------------------------------------------------------------------
// XClipWindow class

class EXTENSION_API XClipWindow : public XTabGroupWindow
{
	DECLARE_CLASS(XClipWindow, XTabGroupWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XClipWindow)

	public:
		XClipWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

	protected:
		INT   childH;
		INT   childV;

		INT   prefHUnits;
		INT   prefVUnits;

		BITFIELD bForceChildWidth:1 GCC_PACK(4);
		BITFIELD bForceChildHeight:1;

		BITFIELD bSnapToUnits:1;
		BITFIELD bFillWindow:1;

	private:
		INT   areaHSize GCC_PACK(4);
		INT   areaVSize;
		INT   childHSize;
		INT   childVSize;

		FLOAT hMult;
		FLOAT vMult;

	public:
		// XClipWindow interface
		void SetChildPosition(INT newX, INT newY);
		void GetChildPosition(INT *pNewX=NULL, INT *pNewY=NULL);

		void SetUnitSize(INT hUnits, INT vUnits);
		void SetUnitWidth(INT hUnits);
		void SetUnitHeight(INT vUnits);
		void ResetUnitSize(void);
		void ResetUnitWidth(void);
		void ResetUnitHeight(void);
		void GetUnitSize(INT *pAreaHSize=NULL,  INT *pAreaVSize=NULL,
		                 INT *pChildHSize=NULL, INT *pChildVSize=NULL);

		void ForceChildSize(UBOOL bNewForceChildWidth=TRUE,
		                    UBOOL bNewForceChildHeight=TRUE);

		void EnableSnapToUnits(UBOOL bNewSnapToUnits=TRUE);

		XWindow *GetChild(void);
		void    QueryClipPreferredSize(UBOOL bWidthSpecified, FLOAT queryWidth,
		                               FLOAT *pPreferredWidth, UBOOL *pBHClip,
		                               UBOOL bHeightSpecified, FLOAT queryHeight,
		                               FLOAT *pPreferredHeight, UBOOL *pBVClip);

		// XWindow interface callbacks
		UBOOL   ChildRequestedReconfiguration(XWindow *pChild);
		void    ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		void    ChildRequestedShowArea(XWindow *pChild, FLOAT showX, FLOAT showY,
		                               FLOAT showWidth, FLOAT showHeight);
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void    ConfigurationChanged(void);

	private:
		// Internal routines used by this class
		void    ReconfigureChild(XWindow *pChild,
		                         INT   childH,     INT childV,
		                         FLOAT childWidth, FLOAT childHeight);
		void    GetChildPreferredSize(XWindow *pChild,
		                              UBOOL bWidthSpecified, FLOAT queryWidth,
		                              UBOOL bHeightSpecified, FLOAT queryHeight);
		void    GetClipPreferredSize(XWindow *pChild,
		                             UBOOL bWidthSpecified,
		                             FLOAT &preferredWidth,
		                             UBOOL bHeightSpecified,
		                             FLOAT &preferredHeight);
		void    GetChildUnits(XWindow *pChild, FLOAT &hUnits, FLOAT &vUnits);
		void    ClampChildPosition(FLOAT &newChildX, FLOAT &newChildY,
		                           FLOAT newChildWidth, FLOAT newChildHeight);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetChildPosition)
		DECLARE_FUNCTION(execGetChildPosition)
		DECLARE_FUNCTION(execSetUnitSize)
		DECLARE_FUNCTION(execSetUnitWidth)
		DECLARE_FUNCTION(execSetUnitHeight)
		DECLARE_FUNCTION(execResetUnitSize)
		DECLARE_FUNCTION(execResetUnitWidth)
		DECLARE_FUNCTION(execResetUnitHeight)
		DECLARE_FUNCTION(execGetUnitSize)
		DECLARE_FUNCTION(execForceChildSize)
		DECLARE_FUNCTION(execEnableSnapToUnits)
		DECLARE_FUNCTION(execGetChild)

};  // XClipWindow


#endif // _EXT_CLIP_WINDOW_H_

