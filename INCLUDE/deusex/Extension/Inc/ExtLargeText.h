
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtLargeText.h
//  Programmer  :  Scott Martin
//  Description :  Header file for large text widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_LARGE_TEXT_H_
#define _EXT_LARGE_TEXT_H_


// ----------------------------------------------------------------------
// Information about a single row of text

struct XTextRowData
{
	FLOAT      xExtent;      // Total length of line (in pixels)
	INT        startPos;     // Starting offset into the string
	INT        count;        // Character count, excluding trailing spaces
	INT        totalCount;   // Character count, including trailing spaces
	XTextState textState;    // Drawn text state at the beginning of the line
};


// ----------------------------------------------------------------------
// Information about a list of text rows

struct XTextParams
{
	friend class XLargeTextWindow;
	private:
		BITFIELD bDirty:1 GCC_PACK(4);       // TRUE if the rows should be regenerated
		BITFIELD bSpecialText:1;             // TRUE if meta-characters are allowed
		INT      dirtyStart GCC_PACK(4);     // Starting position of "dirty" text
		INT      dirtyCount;                 // Ending position of "dirty" text
		UFont    *normalFont;                // Current normal font
		UFont    *boldFont;                  // Current bold font
		FLOAT    destWidth;                  // Width used to compute word-wrap
};


// ----------------------------------------------------------------------
// XLargeTextWindow class

class EXTENSION_API XLargeTextWindow : public XTextWindow
{
	DECLARE_CLASS(XLargeTextWindow, XTextWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XLargeTextWindow)

	public:
		XLargeTextWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		// Spacing
		FLOAT                vSpace;       // Number of pixels between lines

		// Internally computed stuff
		FLOAT                lineHeight;   // Computed line height

		// Row information for current window size
		XTextParams          textParams;
		TArray<XTextRowData> rowData;

		// Row information for last queried window size
		XTextParams          queryTextParams;
		TArray<XTextRowData> queryRowData;

		// Temporary row information
		TArray<XTextRowData> tempRowData;

	public:
		// XLargeTextWindow interface
		void SetVerticalSpacing(FLOAT newVSpace=1);

		// XTextWindow interface
		void SetText(const TCHAR *newText);
		void AppendText(const TCHAR *newText);

		// XWindow interface callbacks
		void ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                  UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void ParentRequestedGranularity(FLOAT &hGranularity, FLOAT &vGranularity);
		void ConfigurationChanged(void);
		void Draw(XGC *gc);

	protected:
		void MakeLinesDirty(void);
		void MakeAreaDirty(INT startPos, INT oldSize, INT newSize);

	private:
		FLOAT ComputeLineHeight(XGC *gc);
		void  GenerateLines(XGC *gc, XTextParams &textParams, TArray<XTextRowData> &rowData,
		                    FLOAT &lineHeight, FLOAT destWidth);
		void  GetTextExtent(TArray<XTextRowData> &rowData, FLOAT &lineHeight,
		                    FLOAT &xExtent, FLOAT &yExtent);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetVerticalSpacing)

};  // XLargeTextWindow


#endif // _EXT_LARGE_TEXT_H_
