
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtRadioBox.h
//  Programmer  :  Scott Martin
//  Description :  Header file for radio boxes
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_RADIO_BOX_H_
#define _EXT_RADIO_BOX_H_


// ----------------------------------------------------------------------
// XRadioBoxWindow class

class EXTENSION_API XRadioBoxWindow : public XTabGroupWindow
{
	DECLARE_CLASS(XRadioBoxWindow, XTabGroupWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XRadioBoxWindow)

	public:
		XRadioBoxWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

		BITFIELD bOneChecked:1 GCC_PACK(4);

	private:
		TArray<XToggleWindow *> toggleButtons GCC_PACK(4);
		XToggleWindow *currentSelection;

	public:
		// XRadioBox interface
		XToggleWindow *GetEnabledToggle(void) { return (currentSelection); }

		// XWindow interface callbacks
		void  ConfigurationChanged(void);
		void  ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                   UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void  ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		UBOOL ChildRequestedReconfiguration(XWindow *pChild);

		UBOOL ToggleChanged(XWindow *pChild, UBOOL bNewState);
		void  DescendantAdded(XWindow *pChild);
		void  DescendantRemoved(XWindow *pChild);

		// Intrinsic routines
		DECLARE_FUNCTION(execGetEnabledToggle)

};  // XRadioBoxWindow


#endif // _EXT_RADIO_BOX_H_
