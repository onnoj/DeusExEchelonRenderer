// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtComputerWindow.h
//  Programmer  :  Albert Yarusso
//  Description :  Header file for Unreal Computer Window widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_COMPUTERWINDOW_H_
#define _EXT_COMPUTERWINDOW_H_

struct DisplayCharacterInfo 
{
	TCHAR character;
	INT posX;
	INT posY;
	FLOAT timeDisplayed;	
};

struct QueuedCharacterInfo  
{
	TCHAR character;
	INT  posX;
	INT  posY;
	FLOAT timeUntilDisplay;
};

enum ECompWinEventTypes
{
	ET_Text			= 0,
	ET_TextColor	= 1,
	ET_TextSound	= 2,
	ET_TextPosition = 3,
	ET_NewLine		= 4,
	ET_Graphic		= 5,
	ET_Clear		= 6,
	ET_Sound		= 7,
	ET_Wait			= 8,
	ET_GetInput		= 9,
	ET_GetChar		= 10,
	ET_FadeOut		= 11
};

struct CompWinEventStruct
{
	ECompWinEventTypes eventType;
	FLOAT eventTime;

	union
	{
		struct		// ET_GetInput, ET_GetChar
		{
			INT inputLength;
			TCHAR *inputKey;
			TCHAR inputMask[2];
			TCHAR *defaultInputString;
			BITFIELD bEcho:1 GCC_PACK(4);
		};
		struct		// ET_Text
		{
			TCHAR character;
			FColor charColor;
			INT  charX;
			INT  charY;
			BITFIELD bNoFade:1 GCC_PACK(4);
		};
		struct		// ET_TextColor
		{
			FColor textColor;
		};
		struct		// ET_TextSound
		{
			USound *textSound;
		};
		struct		// ET_TextPosition
		{
			INT newTextX;
			INT newTextY;
		};
		struct		// ET_NewLine
		{
			BYTE returnPlaceholder;
		};
		struct		// ET_Graphic
		{
			UTexture *graphic;
			INT	graphicX;
			INT graphicY;
			INT width;
			INT height;
			BITFIELD bStatic:1 GCC_PACK(4);
			BITFIELD bPixelPos:1;
		};
		struct		// ET_Clear
		{
			BYTE clearPlaceholder;
		};
		struct		// ET_Sound
		{
			USound *sound;
		};
		struct		// ET_FadeOut
		{
			FLOAT fadeOutTimer;
		};
	};
};

#define QUEUED_BUFFER_START_MAX	4096

// ----------------------------------------------------------------------
// XComputerWindow class

class EXTENSION_API XComputerWindow: public XWindow
{
	DECLARE_CLASS(XComputerWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XComputerWindow)

	public:
		XComputerWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);
		
	protected:
		void FreeQueuedBuffer(void);

	protected:
		FLOAT	eventTimeInterval;			// Timing between each event displayed
		FLOAT	timeNextEvent;				// Time of next event
		FLOAT	timeLastEvent;				// Time last event was added
		FLOAT	timeCurrent;				// Current event time
		FLOAT	fadeSpeed;					// Fade Speed
		FLOAT	throttle;					// Throttle (speed modifier)
		FLOAT   fadeOutTimer;				// Used to fade the entire screen out
		FLOAT   fadeOutStart;				// Starting fade out value
		INT		textCols;					// Number of text cols (for example, 80)
		INT		textRows;					// Number of text rows (for example, 25)
		UFont	*textFont;					// Font used to display text
		INT		fontWidth;					// Current Font Width
		INT		fontHeight;					// Current Font Height
		FColor	fontColor;					// Current Font Color, used when adding text events
		USound	*textSound;					// Sound to play for each character
		USound	*typingSound;				// Sound when user types
		FLOAT	computerSoundVolume;		// Sound Volume
		BITFIELD	bWordWrap:1 GCC_PACK(4);	// True if Word Wrap is enabled
		BITFIELD	bLastLineWrapped:1;			// True if the last line was wrapped
		BITFIELD	bInvokeComputerStart:1;		// True if we need to invoke ComputerStart
		BITFIELD	bComputerStartInvoked:1;	// True if we've invoked ComputerStart event
		BITFIELD	bFirstTick:1;				// True until we've had our first Tick event

		UTexture *backgroundTextures[6] GCC_PACK(4);	// Background textures	

		UTexture *cursorTexture;			// Cursor texture, drawn ahead of text
		FColor   cursorColor;				// Cursor Color
		INT	     cursorWidth;				// Cursor Width
		INT		 cursorHeight;				// Cursor Height
		FLOAT    cursorBlinkSpeed;			// Cursor Blink Speed
		BITFIELD	 bCursorVisible:1 GCC_PACK(4);	// True if Cursor Visible
		BITFIELD	 bShowCursor:1;					// Set to False to hide cursor
		FLOAT	 cursorNextEvent GCC_PACK(4);		// Countdown until 0 for next cursor blink change
		FColor	 colGraphicTile;					// Color used to draw graphics

		XWindow  *textWindow;				// Window that text is displayed in
		APlayerPawnExt *player;				// Pointer to player pawn

		TArray<CompWinEventStruct> displayBuffer;		// Events to display
		TArray<CompWinEventStruct> queuedBuffer;		// Events queued up, waiting to be triggered

		INT		queuedBufferStart;				// Start of queued buffer we're interested in
		INT		textX;							// Current text X position
		INT		textY;							// Current text Y position
	
		// Input related stuff
		FStringNoInit inputKey;						// Input Key
		FStringNoInit inputMask;					// Input Mask Character
		XEditWindow *inputWindow;				// Input Window
		BITFIELD	bWaitingForKey:1 GCC_PACK(4);	// True if we're waiting for a single key
		BITFIELD bEchoKey:1;						// True if echo keypress
		BITFIELD bPauseProcessing:1;				// True if we're pausing processing
		BITFIELD bIgnoreTick:1;					// True if ignoring TICK event
		BITFIELD bGamePaused:1;					// True if the Game is paused
		BITFIELD bIgnoreGamePaused:1;			// True if we're going to ignore the game being paused

		void CalculateTextWindowSize( void );
		void CreateTextWindow( void );
		void CreateInputControl( int queuedIndex );
		void WaitForKey( int queuedIndex );
		void ResetQueuedBuffer( void );
		void ProcessDisplayBuffer( FLOAT deltaSeconds );
		void Scroll( void );
		void ProcessNewLineEvent( void );
		void ProcessQueuedBuffer( void );
		void ProcessCursorBlink( FLOAT deltaSeconds );
		void DrawDisplayBuffer(XGC *gc);

		INT  AddEvent( ECompWinEventTypes eventType );
		void AddCharacterEvent( TCHAR currentChar, INT posX = -1, INT posY = -1, UBOOL bNoFade = FALSE );
		void AddTextColorEvent( FColor textColor );
		void AddTextSoundEvent( USound *newTextSound );
		void AddTextPositionEvent( INT newPosX, INT newPosY );
		void AddNewLineEvent( void );
		void AddGraphicEvent( 
			UTexture *graphic, 
			INT width, 
			INT height, 
			INT posX = -1, 
			INT posY = -1, 
			UBOOL bStatic = FALSE, 
			UBOOL bPixelPos = FALSE);
		void AddClearScreenEvent( void );
		void AddSoundEvent( USound *sound );
		void AddGetInputEvent( 
			INT inputLength, 
			FString &newInputKey, 
			FString &defaultInputString,
			FString &inputMask
			);
		void AddGetCharEvent( FString &newInputKey, UBOOL bEcho = TRUE );
		void AddGetRawKeypress( FString &newInputKey );
		void AddFadeOutEvent( FLOAT newFadeOutTimer );
	
		UBOOL CalculateCharDisplayPosition( 
			INT &queuedIndex, INT &newPosX, INT &newPosY);

		void CalculateGraphicDisplayPosition( INT displayIndex );
		void PlaySoundNow( USound *sound );
		void SetComputerSoundVolume( FLOAT newSoundVolume );
		void SetTypingSoundVolume( FLOAT newSoundVolume );
		void FadeOutText(FLOAT fadeDuration = 2.0);

	public:
		// XComputerWindow interface
		void SetBackgroundTextures( 
			UTexture *backTexture1=NULL,
			UTexture *backTexture2=NULL,
			UTexture *backTexture3=NULL,
			UTexture *backTexture4=NULL,
			UTexture *backTexture5=NULL,
			UTexture *backTexture6=NULL);

		void SetTextSize( INT newCols, INT newRows);
		void SetTextWindowPosition( INT newX, INT newY );

		void SetTextFont( 
			UFont *newFont, 
			INT newFontWidth, 
			INT newFontHeight,
			FColor newFontColor);

		void SetTextColor( FColor newColor );
		void SetTextTiming( FLOAT newTiming );
		void SetFadeSpeed( FLOAT newFadeSpeed );
		void SetCursorTexture( UTexture *newCursorTexture, INT newCursorWidth, INT newCursorHeight );
		void SetCursorColor( FColor newColor );
		void SetCursorBlinkSpeed( FLOAT newBlinkSpeed );
		void ShowTextCursor( UBOOL bShow = TRUE );
		void SetTextSound( USound *newTextSound );
		void SetTypingSound( USound *newTypingSound );
		void ClearScreen( void );
		void ClearLine( INT rowToClear );
		void Print( FString &printText, UBOOL bNewLine = TRUE );
		void PrintLn( void );
		void PrintImmediate( FString &printText );
		void GetInput( INT maxLength, FString &newInputKey, FString &defaultInputString, FString &inputMask);
		void GetChar( FString &newInputKey, UBOOL bEcho = TRUE );
		void PrintGraphic( 
			UTexture *graphic, 
			INT width = -1, 
			INT height = -1, 
			INT posX = -1, 
			INT posY = -1, 
			UBOOL bStatic = FALSE, 
			UBOOL bPixelPos = FALSE);
		void PlaySoundLater( USound *sound );
		void SetTextPosition( INT newPosX, INT newPosY );
		void Pause( FLOAT newPauseLength = -1 );
		void Resume( void );
		void SetThrottle( FLOAT throttleModifier );
		FLOAT GetThrottle( void );
		void ResetThrottle( void );

		UBOOL IsPaused( void );
		void EnableWordWrap( UBOOL bNewWordWrap = TRUE );

		UBOOL IsBufferFlushed( void );

		// XWindow interface callbacks
		void Draw(XGC *gc);
		void Tick(FLOAT deltaSeconds);
		void VisibilityChanged(UBOOL bNewVisibility);
		UBOOL EditActivated(XWindow *pEdit, UBOOL bModified);
		UBOOL VirtualKeyPressed(EInputKey key, UBOOL bRepeat);

		void InvokeComputerStart( void );
		void InvokeComputerInputFinished( FString &newInputKey, FString &inputValue );
		void InvokeComputerFadeOutCompleted(void);

		// Intrinsics
		DECLARE_FUNCTION(execSetBackgroundTextures)
		DECLARE_FUNCTION(execSetTextSize)
		DECLARE_FUNCTION(execSetTextWindowPosition)
		DECLARE_FUNCTION(execSetTextFont)
		DECLARE_FUNCTION(execSetFontColor)
		DECLARE_FUNCTION(execSetTextTiming)
		DECLARE_FUNCTION(execSetFadeSpeed)
		DECLARE_FUNCTION(execSetCursorTexture)
		DECLARE_FUNCTION(execSetCursorColor)
		DECLARE_FUNCTION(execSetCursorBlinkSpeed)
		DECLARE_FUNCTION(execShowTextCursor)
		DECLARE_FUNCTION(execSetTextSound)
		DECLARE_FUNCTION(execSetTypingSound)
		DECLARE_FUNCTION(execSetComputerSoundVolume)
		DECLARE_FUNCTION(execSetTypingSoundVolume)
		DECLARE_FUNCTION(execClearScreen)
		DECLARE_FUNCTION(execClearLine)
		DECLARE_FUNCTION(execPrint)
		DECLARE_FUNCTION(execPrintLn)
		DECLARE_FUNCTION(execGetInput)
		DECLARE_FUNCTION(execGetChar)
		DECLARE_FUNCTION(execPrintGraphic)
		DECLARE_FUNCTION(execPlaySoundLater)
		DECLARE_FUNCTION(execSetTextPosition)
		DECLARE_FUNCTION(execIsBufferFlushed)
		DECLARE_FUNCTION(execPause)
		DECLARE_FUNCTION(execResume)
		DECLARE_FUNCTION(execIsPaused)
		DECLARE_FUNCTION(execSetThrottle)
		DECLARE_FUNCTION(execGetThrottle)
		DECLARE_FUNCTION(execResetThrottle)
		DECLARE_FUNCTION(execEnableWordWrap)
		DECLARE_FUNCTION(execFadeOutText)

};  // XComputerWindow


#endif // _EXT_COMPUTERWINDOW_H_
