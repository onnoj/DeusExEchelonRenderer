
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtText.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal text widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_TEXT_H_
#define _EXT_TEXT_H_


// ----------------------------------------------------------------------
// XTextWindow class

class EXTENSION_API XTextWindow : public XWindow
{
	DECLARE_CLASS(XTextWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XTextWindow)

	public:
		XTextWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		// Text drawing options
		BYTE     hAlign;                   // Horizontal alignment (left, center, right justified)
		BYTE     vAlign;                   // Vertical alignment   (top, center, bottom justified)
		BITFIELD bWordWrap:1 GCC_PACK(4);  // Is word wrapping on?
		BITFIELD bTextIsAccelerator:1;     // Is our text also used for keyboard acceleration?

		// Margins
		FLOAT   hMargin GCC_PACK(4);       // Horizontal margin between the window and the text
		FLOAT   vMargin;                   // Vertical margin between the window and the text

		// Preferred sizes
		INT     minLines;      // Preferred maximum number of lines; -1 == no preferred size
		INT     maxLines;      // Preferred minimum number of lines; -1 == no preferred size
		FLOAT   minWidth;      // Preferred minimum width; 0 == no preferred min width

		FStringNoInit text;         // Text string to draw

	public:
		// XTextWindow interface
		virtual void SetText(const TCHAR *newText);
		virtual void AppendText(const TCHAR *newText);
		const TCHAR *GetText(void)  { return (*text); }

		void SetWordWrap(UBOOL bNewWordWrap);
		void SetTextAlignments(EHAlign newHAlign, EVAlign newVAlign);
		void SetTextMargins(FLOAT newHMargin, FLOAT newVMargin);
		void SetLines(INT newMinLines, INT newMaxLines);
		void SetMinLines(INT newMinLines);
		void SetMaxLines(INT newMaxLines);

		void ResetLines(void);
		void SetMinWidth(FLOAT minWidth);
		void ResetMinWidth(void);

		void EnableTextAsAccelerator(UBOOL bEnable=TRUE);

		// XWindow interface callbacks
		void ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                  UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void ParentRequestedGranularity(FLOAT &hGranularity, FLOAT &vGranularity);
		void Draw(XGC *gc);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetText)
		DECLARE_FUNCTION(execAppendText)
		DECLARE_FUNCTION(execGetText)
		DECLARE_FUNCTION(execGetTextLength)
		DECLARE_FUNCTION(execGetTextPart)
		DECLARE_FUNCTION(execSetWordWrap)
		DECLARE_FUNCTION(execSetTextAlignments)
		DECLARE_FUNCTION(execSetTextMargins)
		DECLARE_FUNCTION(execSetLines)
		DECLARE_FUNCTION(execSetMinLines)
		DECLARE_FUNCTION(execSetMaxLines)
		DECLARE_FUNCTION(execResetLines)
		DECLARE_FUNCTION(execSetMinWidth)
		DECLARE_FUNCTION(execResetMinWidth)
		DECLARE_FUNCTION(execEnableTextAsAccelerator)

};  // XTextWindow


#endif // _EXT_TEXT_H_
