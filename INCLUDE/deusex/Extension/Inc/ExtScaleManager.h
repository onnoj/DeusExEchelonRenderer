
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtScaleManager.h
//  Programmer  :  Scott Martin
//  Description :  Header file for scale manager window class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_SCALE_MANAGER_H_
#define _EXT_SCALE_MANAGER_H_


// ----------------------------------------------------------------------
// XScaleManagerWindow class

class EXTENSION_API XScaleManagerWindow : public XWindow
{
	DECLARE_CLASS(XScaleManagerWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XScaleManagerWindow)

	public:
		XScaleManagerWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

	protected:
		XButtonWindow *decButton;
		XButtonWindow *incButton;
		XTextWindow   *valueField;
		XScaleWindow  *scale;

		EOrientation  orientation;

		BITFIELD      bStretchScaleField:1 GCC_PACK(4);
		BITFIELD      bStretchValueField:1;

		FLOAT         marginWidth GCC_PACK(4);
		FLOAT         marginHeight;
		FLOAT         spacing;

		BYTE          childHAlign;
		BYTE          childVAlign;

	public:
		// XScaleManagerWindow interface
		void SetScaleButtons(XButtonWindow *newDecButton=NULL, XButtonWindow *newIncButton=NULL);
		void SetValueField(XTextWindow *newValueField=NULL);
		void SetScale(XScaleWindow *newScale=NULL);

		void SetOrientation(EOrientation newOrientation)
		{
			orientation = newOrientation;
			AskParentForReconfigure();
		}

		void StretchScaleField(UBOOL bNewStretch=TRUE)
		{
			bStretchScaleField = bNewStretch;
			AskParentForReconfigure();
		}
		void StretchValueField(UBOOL bNewStretch=TRUE)
		{
			bStretchValueField = bNewStretch;
			AskParentForReconfigure();
		}

		void SetManagerMargins(FLOAT newMarginWidth, FLOAT newMarginHeight)
		{
			if ((marginWidth != newMarginWidth) || (marginHeight != newMarginHeight))
			{
				marginWidth  = newMarginWidth;
				marginHeight = newMarginHeight;
				AskParentForReconfigure();
			}
		}
		void SetManagerSpacing(FLOAT newSpacing)
		{
			if (spacing != newSpacing)
			{
				spacing = newSpacing;
				AskParentForReconfigure();
			}
		}

		void SetManagerAlignments(EHAlign newHAlign, EVAlign newVAlign)
		{
			if ((childHAlign != newHAlign) || (childVAlign != newVAlign))
			{
				childHAlign = newHAlign;
				childVAlign = newVAlign;
				AskParentForReconfigure();
			}
		}

		// XWindow interface callbacks
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		UBOOL   ChildRequestedReconfiguration(XWindow *pChild);
		void    ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		void    ConfigurationChanged(void);

		void    ChildRemoved(XWindow *pChild);

		UBOOL   ScalePositionChanged(XWindow *pScale, INT newTickPosition, FLOAT newValue,
		                             UBOOL bFinal);
		UBOOL   ScaleAttributesChanged(XWindow *pScale,
		                               INT tickPosition, INT tickSpan,
		                               INT numTicks);

		UBOOL   ButtonActivated(XWindow *pButton);

	private:
		// Internal routines used by this class
		void AddToBoundingBox(FLOAT &boxWidth, FLOAT &boxHeight,
		                      FLOAT subWidth,  FLOAT subHeight,
		                      FLOAT distance);

		void ComputeChildConfig(XWindow *child, FLOAT &curPos,
		                        FLOAT distance);
		void ComputeChildOffset(XWindow *child, FLOAT &curAdd,
		                        FLOAT addSize);

		void  ChangeValueField(void);
		UBOOL IsDescendant(XWindow *pChild);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetScaleButtons)
		DECLARE_FUNCTION(execSetValueField)
		DECLARE_FUNCTION(execSetScale)
		DECLARE_FUNCTION(execSetManagerOrientation)
		DECLARE_FUNCTION(execStretchScaleField)
		DECLARE_FUNCTION(execStretchValueField)
		DECLARE_FUNCTION(execSetManagerMargins)
		DECLARE_FUNCTION(execSetManagerSpacing)
		DECLARE_FUNCTION(execSetManagerAlignments)

};  // XScaleManagerWindow


#endif // _EXT_SCALE_MANAGER_H_
