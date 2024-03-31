
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtEdit.h
//  Programmer  :  Scott Martin
//  Description :  Header file for edit widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_EDIT_H_
#define _EXT_EDIT_H_


// ----------------------------------------------------------------------
// Undo buffer structure

struct XUndoBuffer
{
	friend class XEditWindow;
	~XUndoBuffer()
	{
		oldString.Empty();
		newString.Empty();
	}
	private:
		INT     insertPos;
		FString oldString;
		FString newString;
};


// ----------------------------------------------------------------------
// XEditWindow class

class EXTENSION_API XEditWindow : public XLargeTextWindow
{
	DECLARE_CLASS(XEditWindow, XLargeTextWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XEditWindow)

	public:
		XEditWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		// Edit options
		BITFIELD bEditable:1 GCC_PACK(4); // TRUE if the window is editable
		BITFIELD bSingleLine:1;           // TRUE if single-line editing is enabled
		BITFIELD bUppercaseOnly:1;        // TRUE if only uppercase letters are allowed

		// Insertion point information
		INT      insertPos GCC_PACK(4);   // Insertion point position (text offset)
		INT      insertHookPos;           // Insertion hook point; used for dragging
		BYTE     insertType;              // Insertion point cursor type

		// Selection information
		INT      selectStart;       // Starting position of selected text
		INT      selectEnd;         // Ending position of selected text

		// Text options
		INT      maxSize;           // Maximum size of edited text (0=infinite)

		// Appearance information
		UTexture *insertTexture;    // Texture used for the insertion point
		FColor   insertColor;       // Color used for the insertion point
		UTexture *selectTexture;    // Texture used for selected areas
		FColor   selectColor;       // Color used for selected areas
		FColor   inverseColor;      // Color used for selected text

		// Cursor data
		XCursor  *editCursor;       // Edit cursor (NULL for default)
		UTexture *editCursorShadow;
		FColor   editCursorColor;

		// Sounds
		USound   *typeSound;        // Sound played when typing
		USound   *deleteSound;      // Sound played when deleting
		USound   *enterSound;       // Sound played when Enter is pressed
		USound   *moveSound;        // Sound played when the cursor is moved

	private:
		// Undo buffer
		TArray<XUndoBuffer> bufferList;     // Undo buffer
		INT                 currentUndo;    // Current undo position
		INT                 maxUndos;       // Maximum level of undos
		INT                 unchangedUndo;  // Location of "unchanged" undo

		// Timers
		FLOAT    dragDelay;                     // Amount of time remaining until next drag
		FLOAT    blinkDelay;                    // Remaining time for insertion point visibility
		FLOAT    blinkStart;                    // Initial amount of time before blinking starts
		FLOAT    blinkPeriod;                   // Amount of time to cycle through one blink
		BITFIELD bInsertShowing:1 GCC_PACK(4);  // TRUE if the insertion point is "blinked on"

		// Selection options
		BITFIELD bDragging:1;       // TRUE if the user is dragging the insertion point
		BITFIELD bSelectWords:1;    // TRUE if the user is selecting words

		// Insertion point configuration
		FLOAT    insertX GCC_PACK(4);  // X position of insertion point
		FLOAT    insertY;              // Y position of insertion point
		FLOAT    insertWidth;          // Width of insertion point
		FLOAT    insertHeight;         // Height of insertion point
		FLOAT    insertPrefWidth;      // User-specified insertion point width
		FLOAT    insertPrefHeight;     // User-specified insertion point height
		FLOAT    showAreaX;            // X position of area to be displayed
		FLOAT    showAreaY;            // Y position of area to be displayed
		FLOAT    showAreaWidth;        // Width of area to be displayed
		FLOAT    showAreaHeight;       // Height of area to be displayed

		// Insertion point column
		FLOAT    insertPreferredCol;   // Preferred column position (in pixels)

		// Last configured size
		FLOAT    lastConfigWidth;   // Last configured width
		FLOAT    lastConfigHeight;  // Last configured height

		// Selection position information
		INT      selectStartRow;    // Row for the start of the selected area
		INT      selectEndRow;      // Row for the end of the selected area
		FLOAT    selectStartX;      // Pixel offset for selected area start
		FLOAT    selectEndX;        // Pixel offset for selected area end

	public:
		// XEditWindow interface
		void  MoveInsertionPoint(EMoveInsert moveInsert, UBOOL bDrag=FALSE);
		void  SetInsertionPoint(INT newInsertionPoint, UBOOL bDrag=FALSE);
		INT   GetInsertionPoint(void)  { return (insertPos); }
		void  SetSelectedArea(INT startPos, INT count);
		void  GetSelectedArea(INT *pStartPos=NULL, INT *pCount=NULL);

		void  EnableEditing(UBOOL bEdit=TRUE);
		UBOOL IsEditingEnabled(void)  { return (bEditable); }

		void  EnableSingleLineEditing(UBOOL bSingleLine);
		UBOOL IsSingleLineEditingEnabled(void)  { return (bSingleLine); }

		void  EnableUppercaseOnly(UBOOL bUppercaseOnly=TRUE);

		void  ClearTextChangedFlag(void);
		void  SetTextChangedFlag(UBOOL bTextChanged=TRUE);
		UBOOL HasTextChanged(void)  { return (currentUndo!=unchangedUndo); }

		void  SetMaxSize(INT newMaxSize);
		void  SetMaxUndos(INT newMaxUndos);

		virtual UBOOL InsertText(const TCHAR *insertStr=NULL,
		                         UBOOL bUndo=FALSE, UBOOL bSelect=FALSE);
		virtual void DeleteChar(UBOOL bBefore=FALSE, UBOOL bUndo=FALSE);

		void  SetInsertionPointBlinkRate(FLOAT newBlinkStart=0.75,
		                                 FLOAT newBlinkPeriod=1.0);

		void  SetInsertionPointTexture(UTexture *newTexture=NULL,
		                               FColor newColor=FColor(255, 255, 255));
		void  SetInsertionPointType(EInsertionPointType newType,
		                            FLOAT prefWidth=0, FLOAT prefHeight=0);
		void  SetSelectedAreaTexture(UTexture *newTexture,
		                             FColor newColor=FColor(192, 192, 192));
		void  SetSelectedAreaTextColor(FColor newColor=FColor(0, 0, 0));

		void  SetEditCursor(XCursor *newCursor=NULL, UTexture *newCursorShadow=NULL, 
			                FColor newColor=FColor(255, 255, 255));

		void  Undo(void);
		void  Redo(void);
		void  ClearUndo(void);

		void  Copy(void);
		void  Cut(void);
		void  Paste(void);

		void  SetEditSounds(USound *typeSound=NULL, USound *deleteSound=NULL,
		                    USound *enterSound=NULL, USound *moveSound=NULL);
		void  PlayEditSound(USound *sound, FLOAT volume=-1.0, FLOAT pitch=1.0);

		// XTextWindow interface
		void  SetText(const TCHAR *newText);
		void  AppendText(const TCHAR *newText);

		// XEditWindow interface callbacks
		virtual UBOOL FilterChar(TCHAR &ch);

		// XWindow interface callbacks
		XCursor *CursorRequested(XWindow *win, FLOAT pointX, FLOAT pointY,
		                         FLOAT &hotX, FLOAT &hotY, FColor &newColor,
								 UTexture **pCursorShadow);
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified, FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void    ConfigurationChanged(void);
		void    VisibilityChanged(UBOOL bNewVisibility);
		void    Draw(XGC *gc);
		void    Tick(FLOAT deltaSeconds);
		UBOOL   KeyPressed(TCHAR key);
		UBOOL   VirtualKeyPressed(EInputKey key, UBOOL bRepeat);
		UBOOL   MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                           INT numClicks);
		void    MouseMoved(FLOAT pointX, FLOAT pointY);
		UBOOL   MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                            INT numClicks);

	protected:
		void  CopyToTextBuffer(UBOOL bCut=FALSE, UBOOL bUndo=FALSE);
		void  PasteFromTextBuffer(UBOOL bUndo=FALSE, UBOOL bSelect=FALSE);

		INT   YToRow(FLOAT pointY);
		INT   XToCol(INT row, FLOAT pointX);
		FLOAT RowToY(INT row);
		FLOAT ColToX(INT row, INT col);
		INT   RowColToPos(INT row, INT col);
		void  PosToRowCol(INT pos, INT *pRow, INT *pCol);
		INT   XYToPos(FLOAT pointX, FLOAT pointY);
		void  PosToXY(INT pos, FLOAT *pPointX, FLOAT *pPointY);

	private:
		void  ReplaceText(const TCHAR *newStr, UBOOL bUndo=FALSE, UBOOL bSelect=FALSE);
		void  DeleteText(UBOOL bBefore=TRUE, UBOOL bUndo=FALSE);

		void  ComputeCursorConfig(UBOOL bSetPrefCol=TRUE);
		void  ShowCursor(void);
		void  SetCursorPoint(INT pos, UBOOL bDrag);
		void  SetCursorRow(INT row, UBOOL bDrag);
		INT   NextWord(INT startPos);
		INT   PrevWord(INT startPos);
		INT   StartOfWord(INT startPos);
		INT   EndOfWord(INT endPos);
		void  ComputeSelectArea(void);
		void  CopyToString(FString &destStr, const TCHAR *srcStr, INT size);
		void  AddUndo(INT pos, const TCHAR *oldStr, INT oldCount,
		              const TCHAR *newStr, INT newCount);
		void  ClampUndo(void);
		UBOOL FilterString(const TCHAR *origStr, FString &newStr);
		void  ChangeText(void);
		void  ActivateEdit(void);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execMoveInsertionPoint)
		DECLARE_FUNCTION(execSetInsertionPoint)
		DECLARE_FUNCTION(execGetInsertionPoint)
		DECLARE_FUNCTION(execSetSelectedArea)
		DECLARE_FUNCTION(execGetSelectedArea)

		DECLARE_FUNCTION(execEnableEditing)
		DECLARE_FUNCTION(execIsEditingEnabled)

		DECLARE_FUNCTION(execEnableSingleLineEditing)
		DECLARE_FUNCTION(execIsSingleLineEditingEnabled)

		DECLARE_FUNCTION(execEnableUppercaseOnly)

		DECLARE_FUNCTION(execClearTextChangedFlag)
		DECLARE_FUNCTION(execSetTextChangedFlag)
		DECLARE_FUNCTION(execHasTextChanged)

		DECLARE_FUNCTION(execSetMaxSize)
		DECLARE_FUNCTION(execSetMaxUndos)

		DECLARE_FUNCTION(execInsertText)
		DECLARE_FUNCTION(execDeleteChar)

		DECLARE_FUNCTION(execSetInsertionPointBlinkRate)

		DECLARE_FUNCTION(execSetInsertionPointTexture)
		DECLARE_FUNCTION(execSetInsertionPointType)
		DECLARE_FUNCTION(execSetSelectedAreaTexture)
		DECLARE_FUNCTION(execSetSelectedAreaTextColor)

		DECLARE_FUNCTION(execSetEditCursor)

		DECLARE_FUNCTION(execUndo)
		DECLARE_FUNCTION(execRedo)
		DECLARE_FUNCTION(execClearUndo)

		DECLARE_FUNCTION(execCopy)
		DECLARE_FUNCTION(execCut)
		DECLARE_FUNCTION(execPaste)

		DECLARE_FUNCTION(execSetEditSounds)
		DECLARE_FUNCTION(execPlayEditSound)

};  // XEditWindow


#endif // _EXT_EDIT_H_
