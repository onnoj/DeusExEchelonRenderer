
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtTextLog.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal text logs
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_TEXT_LOG_H
#define _EXT_TEXT_LOG_H_


// ----------------------------------------------------------------------
// XTextLogLine - Private structure which holds one log entry

struct XTextLogLine
{
	FLOAT   timeLeft; // Expiration date
	FString lineStr;  // Log entry
	FColor  lineCol;	// Color of line
};


// ----------------------------------------------------------------------
// XTextLogWindow class

class EXTENSION_API XTextLogWindow : public XTextWindow
{
	DECLARE_CLASS(XTextLogWindow, XTextWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XTextLogWindow)

	public:
		XTextLogWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		FLOAT                textTimeout;    // Time before a log entry disappears

	private:
		// Internal info
		TArray<XTextLogLine> lines;                    // Individual log entries
		BITFIELD             bTooTall:1 GCC_PACK(4);   // TRUE if the log won't fit in the window
		BITFIELD             bAutoVertSize:1;          // TRUE if the log sizes itself vertically
		BITFIELD             bPaused:1;                // TRUE if the log is paused

	public:
		// XTextLogWindow interface
		void AddLog(const TCHAR *newText, FColor linecol);
		void ClearLog(void);
		void SetTextTimeout(FLOAT timeoutSecs);
		void PauseLog(UBOOL bNewPauseState);

		// XWindow interface callbacks
		void  ConfigurationChanged(void);
		void  Draw(XGC *gc);
		void  Tick(FLOAT deltaSeconds);

	private:
		// Internal routines
		void RegenerateText(void);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execAddLog)
		DECLARE_FUNCTION(execClearLog)
		DECLARE_FUNCTION(execSetTextTimeout)
		DECLARE_FUNCTION(execPauseLog)

};  // XTextLogWindow


#endif // _EXT_TEXT_LOG_H_
