
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtButton.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal pushbutton widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_BUTTON_H_
#define _EXT_BUTTON_H_


// ----------------------------------------------------------------------

// Number of button states
#define MAX_BUTTON_STATES  (6)

// Button states are defined as follows:
// 0 - Sensitive,   unfocused, unpressed
// 1 - Sensitive,   unfocused, pressed
// 2 - Sensitive,   focused,   unpressed
// 3 - Sensitive,   focused,   pressed
// 4 - Insensitive, unfocused, unpressed
// 5 - Insensitive, unfocused, pressed
//
// (There are technically two more permutations, insensitive/focused
// pressed and unpressed, but insensitive widgets by definition cannot
// have focus.)


// ----------------------------------------------------------------------
// Structures
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// XButtonDisplayInfo - Information for one button state's display

struct XButtonDisplayInfo
{
	UTexture *texture;
	FColor   tileColor;
	FColor   textColor;
};


// ----------------------------------------------------------------------
// XButtonWindow class

class EXTENSION_API XButtonWindow : public XTextWindow
{
	DECLARE_CLASS(XButtonWindow, XTextWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XButtonWindow)

	public:
		XButtonWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

		// Button/mouse states
		BITFIELD bButtonPressed:1 GCC_PACK(4);   // TRUE if button window is "activated"
		BITFIELD bMousePressed:1;                // TRUE if the mouse button is being pressed
		BITFIELD bAutoRepeat:1;                  // TRUE if auto-repeat is enabled
		BITFIELD bEnableRightMouseClick:1;       // TRUE if allow right mouse click for button activated

		// Button activation variables
		FLOAT    activateDelay GCC_PACK(4);      // Seconds to wait before unpressing button
		FLOAT    initialDelay;                   // Initial delay before repeating
		FLOAT    repeatRate;                     // Amount of time between repeat clicks

	protected:
		UTexture *curTexture;        // Currently displayed texture
		FColor   curTileColor;       // Current tile color
		FColor   curTextColor;       // Current text color

	private:
		// Sounds
		USound   *pressSound;        // Sound played when button is pressed
		USound   *clickSound;        // Sound played when button is released inside window

		XTimerId  activateTimer;     // Timer for unpressing button
		FLOAT     repeatTime;        // Time value for auto-repeat
		BYTE      lastInputKey;      // Last key pressed, used for repeating

		// Button state information
		XButtonDisplayInfo info[MAX_BUTTON_STATES];

	public:
		// XButtonWindow interface
		void ActivateButton(EInputKey key);
		void SetActivateDelay(FLOAT newDelay)  { activateDelay = newDelay; }

		void SetButtonTextures(UTexture *normal=NULL,
		                       UTexture *pressed=NULL,
		                       UTexture *normalFocus=NULL,
		                       UTexture *pressedFocus=NULL,
		                       UTexture *normalInsensitive=NULL,
		                       UTexture *pressedInsensitive=NULL);

		void SetButtonColors(FColor normal=FColor(255,255,255),
		                     FColor pressed=FColor(255,255,255),
		                     FColor normalFocus=FColor(255,255,255),
		                     FColor pressedFocus=FColor(255,255,255),
		                     FColor normalInsensitive=FColor(255,255,255),
		                     FColor pressedInsensitive=FColor(255,255,255));

		void SetTextColors(FColor normal=FColor(0,255,0),
		                   FColor pressed=FColor(0,255,0),
		                   FColor normalFocus=FColor(0,255,0),
		                   FColor pressedFocus=FColor(0,255,0),
		                   FColor normalInsensitive=FColor(0,255,0),
		                   FColor pressedInsensitive=FColor(0,255,0));

		void EnableAutoRepeat(UBOOL bEnable=TRUE,
		                      FLOAT initialDelay=0.5, FLOAT repeatRate=0.1);

		void EnableRightMouseClick(UBOOL bEnable=TRUE);

		void SetButtonSounds(USound *newPressSound=NULL, USound *newReleaseSound=NULL);
		virtual void PressButton(EInputKey key);

		// XWindow interface callbacks
		UBOOL MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                         INT numClicks);
		UBOOL MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                          INT numClicks);
		void  MouseMoved(FLOAT pointX, FLOAT pointY);
		void  SensitivityChanged(UBOOL bNewSensitivity);
		void  FocusEnteredWindow(void);
		void  FocusLeftWindow(void);
		void  Timer(XTimerId timerId, INT invocations, XTimerData clientData);
		void  Tick(FLOAT deltaSeconds);
		void  Draw(XGC *gc);

	protected:
		void ChangeButtonAppearance(void);

	private:
		void AddActivateTimer(void);
		void RemoveActivateTimer(void);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execActivateButton)
		DECLARE_FUNCTION(execSetActivateDelay)
		DECLARE_FUNCTION(execSetButtonTextures)
		DECLARE_FUNCTION(execSetButtonColors)
		DECLARE_FUNCTION(execSetTextColors)
		DECLARE_FUNCTION(execEnableAutoRepeat)
		DECLARE_FUNCTION(execEnableRightMouseClick)
		DECLARE_FUNCTION(execSetButtonSounds)
		DECLARE_FUNCTION(execPressButton)

};  // XButtonWindow


#endif // _EXT_BUTTON_H_
