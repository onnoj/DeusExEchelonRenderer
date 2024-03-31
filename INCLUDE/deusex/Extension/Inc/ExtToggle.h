
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtToggle.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal toggle button widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_TOGGLE_H_
#define _EXT_TOGGLE_H_


// ----------------------------------------------------------------------
// XToggleWindow class

class EXTENSION_API XToggleWindow : public XButtonWindow
{
	DECLARE_CLASS(XToggleWindow, XButtonWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XToggleWindow)

	public:
		XToggleWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

	protected:
		USound *enableSound;
		USound *disableSound;

	public:
		// XToggleWindow interface
		void  ChangeToggle(void);
		void  SetToggle(UBOOL bNewToggle);
		UBOOL GetToggle(void);

		void  SetToggleSounds(USound *enableSound=NULL, USound *disableSound=NULL);

		void  PressButton(EInputKey key);

		// XWindow interface callbacks
		UBOOL MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                         INT numClicks);
		UBOOL MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                          INT numClicks);

		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execChangeToggle)
		DECLARE_FUNCTION(execSetToggle)
		DECLARE_FUNCTION(execGetToggle)
		DECLARE_FUNCTION(execSetToggleSounds)

};  // XToggleWindow


#endif // _EXT_TOGGLE_H_
