
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtList.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal list widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_LIST_H_
#define _EXT_LIST_H_


// ----------------------------------------------------------------------
// XListColData structure - holds data for one column in the list

// BOOGER -- someday, add Serialize() to these structures

struct XListColData
{
	INT      index;
	FString  title;
	INT      sortIndex;
	BITFIELD bReverse:1 GCC_PACK(4);
	BITFIELD bCaseSensitive:1;
	BITFIELD bHide:1;
	FLOAT    colWidth GCC_PACK(4);
	BYTE     alignment;
	FColor   colColor;
	UFont    *colFont;
	BYTE     colType;
	FString  colFmt;

	~XListColData()
	{
		title.Empty();
		colFmt.Empty();
	}
};


// ----------------------------------------------------------------------
// XListFieldData structure - holds data for one field (row AND column)
//                            in the list

struct XListFieldData
{
	FString  field;
	FLOAT    fieldValue;
	FLOAT    fieldWidth;
	FLOAT    fieldHeight;

	~XListFieldData()
	{
		field.Empty();
	}
};


// ----------------------------------------------------------------------
// XListRowData structure - holds data for an entire row in the list
//                          (including all fields)

struct XListRowData
{
	INT                    index;
	BITFIELD               bSelected:1 GCC_PACK(4);
	TArray<XListFieldData> fieldData GCC_PACK(4);
	INT                    clientData;

	~XListRowData()
	{
		fieldData.Empty();
	}
};


// ----------------------------------------------------------------------
// XListWindow class

class EXTENSION_API XListWindow : public XWindow
{
	DECLARE_CLASS(XListWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XListWindow)

	public:
		XListWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		FStringNoInit          delimiter;
		FColor                 inverseColor;
		UTexture               *highlightTexture;
		UTexture               *focusTexture;
		FColor                 highlightColor;
		FColor                 focusColor;
		FLOAT                  focusThickness;
		TArray<XListRowData *> rows;
		TArray<XListColData>   cols;

		BITFIELD               bAutoSort:1 GCC_PACK(4);
		BITFIELD               bAutoExpandColumns:1;
		BITFIELD               bMultiSelect:1;
		FLOAT                  colMargin GCC_PACK(4);
		FLOAT                  rowMargin;

		BITFIELD               bHotKeys:1 GCC_PACK(4);
		INT                    hotKeyCol GCC_PACK(4);

		FLOAT                  lineSize;

		USound                 *activateSound;
		USound                 *moveSound;

	private:
		INT                    numSelected;
		XListRowData           *focusLine;
		XListRowData           *anchorLine;
		BITFIELD               bDragging:1 GCC_PACK(4);
		INT                    lastIndex GCC_PACK(4);
		FLOAT                  remainingDelay;

		FStringNoInit          hotKeyString;
		FLOAT                  hotKeyTimer;

	public:
		// XListWindow interface
		INT  IndexToRowId(INT index);
		INT  RowIdToIndex(INT rowId);

		void SetClientData(INT rowId, INT clientData);
		INT  GetClientData(INT rowId);

		INT  AddRow(const TCHAR *lineStr=NULL, INT clientData=0);
		void DeleteRow(INT rowId);
		void ModifyRow(INT rowId, const TCHAR *lineStr);
		void DeleteAllRows(void);

		void        SetField(INT rowId, INT colIndex, const TCHAR *newField);
		const TCHAR *GetField(INT rowId, INT colIndex);
		void        SetFieldValue(INT rowId, INT colIndex, FLOAT newValue);
		FLOAT       GetFieldValue(INT rowId, INT colIndex);

		INT  GetNumRows(void)  { return (rows.Num()); }
		INT  GetNumSelectedRows(void)  { return (numSelected); }

		void SelectRow(INT rowId, UBOOL bSelected=TRUE);
		void SelectAllRows(UBOOL bSelected=TRUE);

		void SelectToRow(INT rowId, UBOOL bClearRows=TRUE, UBOOL bInvert=FALSE,
		                 UBOOL bDrag=FALSE);
		void ToggleRowSelection(INT rowId);

		UBOOL IsRowSelected(INT rowId);
		INT   GetSelectedRow(void);

		void  MoveRow(EMoveList move, UBOOL bSelect=TRUE, UBOOL bClearRows=TRUE, UBOOL bDrag=FALSE);
		void  SetRow(INT rowId, UBOOL bSelect=TRUE, UBOOL bClearRows=TRUE, UBOOL bDrag=FALSE);

		void SetFocusRow(INT rowId, UBOOL bMoveTo=TRUE, UBOOL bAnchor=TRUE);
		INT  GetFocusRow(void);

		void SetNumColumns(INT numColumns);
		INT  GetNumColumns(void)  { return (cols.Num()); }

		void ResizeColumns(UBOOL bExpandOnly=FALSE);

		void        SetColumnTitle(INT colIndex, const TCHAR *newTitle);
		const TCHAR *GetColumnTitle(INT colIndex);

		void    SetColumnWidth(INT colIndex, FLOAT newColWidth);
		FLOAT   GetColumnWidth(INT colIndex);

		void    SetColumnAlignment(INT colIndex, EHAlign newHAlign);
		EHAlign GetColumnAlignment(INT colIndex);

		void    SetColumnColor(INT colIndex, FColor newColor);
		FColor  GetColumnColor(INT colIndex);

		void    SetColumnFont(INT colIndex, UFont *newFont);
		UFont   *GetColumnFont(INT colIndex);

		void        SetColumnType(INT colIndex, EColumnType colType,
		                          const TCHAR *newFmt=NULL);
		EColumnType GetColumnType(INT colIndex);

		void    HideColumn(INT colIndex, UBOOL bHide=TRUE);
		UBOOL   IsColumnHidden(INT colIndex);

		void SetSortColumn(INT colIndex, UBOOL bReverse=FALSE,
		                   UBOOL bCaseSensitive=FALSE);
		void AddSortColumn(INT colIndex, UBOOL bReverse=FALSE,
		                   UBOOL bCaseSensitive=FALSE);
		void RemoveSortColumn(INT colIndex);
		void ResetSortColumns(UBOOL bSort=TRUE);
		void Sort(void);

		void EnableHotKeys(UBOOL bEnable=TRUE);
		void SetHotKeyColumn(INT colIndex);

		INT  FindRowByKey(TCHAR key);

		void  EnableAutoSort(UBOOL bNewAutoSort=TRUE);
		UBOOL IsAutoSortEnabled(void)  { return (bAutoSort); }

		void  EnableAutoExpandColumns(UBOOL bNewAutoExpand=TRUE);
		UBOOL IsAutoExpandColumnsEnabled(void)  { return (bAutoExpandColumns); }

		void  EnableMultiSelect(UBOOL bNewMultiSelect=TRUE);
		UBOOL IsMultiSelectEnabled(void)  { return (bMultiSelect); }

		void SetFieldMargins(FLOAT newMarginWidth, FLOAT newMarginHeight);
		void GetFieldMargins(FLOAT *pMarginWidth=NULL, FLOAT *pMarginHeight=NULL)
		{
			if (pMarginWidth)  *pMarginWidth  = colMargin;
			if (pMarginHeight) *pMarginHeight = rowMargin;
		}

		INT  GetPageSize(void);

		void SetDelimiter(TCHAR newDelimiter);
		void SetHighlightTextColor(FColor newColor);
		void SetHighlightTexture(UTexture *newTexture);
		void SetHighlightColor(FColor newColor);
		void SetFocusTexture(UTexture *newTexture);
		void SetFocusColor(FColor newColor);
		void SetFocusThickness(FLOAT newThickness);

		void ShowFocusRow(void);

		void SetListSounds(USound *activateSound=NULL, USound *moveSound=NULL);
		void PlayListSound(USound *sound, FLOAT volume=-1.0, FLOAT pitch=1.0);

		// XWindow interface callbacks
		void ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                  UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void ParentRequestedGranularity(FLOAT &hGranularity, FLOAT &vGranularity);
		void Draw(XGC *gc);

		UBOOL   MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                           INT numClicks);
		void    MouseMoved(FLOAT pointX, FLOAT pointY);
		UBOOL   MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                            INT numClicks);

		UBOOL   KeyPressed(TCHAR key);
		UBOOL   VirtualKeyPressed(EInputKey key, UBOOL bRepeat);

		void    VisibilityChanged(UBOOL bVisible);

		void    Tick(FLOAT deltaSeconds);

	private:
		void  InitColumn(XListColData &col, INT index);
		void  RecomputeIndices(void);
		FLOAT StringToFloat(const TCHAR *str);
		FLOAT FieldConvertToValue(BYTE colType, const TCHAR *str);
		const TCHAR *FieldConvertToString(BYTE colType, const TCHAR *colFmt, FLOAT value);
		void  SetFieldByString(XListRowData *row, INT col, const TCHAR *str);
		void  SetFieldByValue(XListRowData *row, INT col, FLOAT value);
		void  ComputeFieldSize(XGC *gc, XListRowData *row, INT col);
		UBOOL ExpandColumnByField(XListRowData *row, INT colIndex);
		void  ComputeRowSize(void);
		void  ExtendRow(XListRowData *row);
		UBOOL FillRow(XListRowData *row, const TCHAR *lineStr);
		XListRowData *CreateRow(const TCHAR *lineStr, INT clientData);
		void  SetupSortedColData(void);
		UBOOL FindRowData(XListRowData *row, INT *pPos);
		void  SortRows(void);
		void  CheckPosition(XListRowData *row);
		void  InsertRow(XListRowData *row);
		void  RemoveRow(XListRowData *row);
		void  ChangeSelectRow(INT rowIndex, UBOOL bSelected, UBOOL &bNotify);
		void  MoveToRow(XListRowData *row, UBOOL bChangeSelect, UBOOL bClearRows,
		                UBOOL bInvert, UBOOL bSpanRange, UBOOL bFocus);
		void  ActivateListRow(void);
		void  ChangeListSelection(void);
		UBOOL IsColumnValid(INT colIndex);
		UBOOL IsHotKeyValid(TCHAR key);
		void  ClearHotKeyString(void);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execIndexToRowId)
		DECLARE_FUNCTION(execRowIdToIndex)
		DECLARE_FUNCTION(execSetRowClientInt)
		DECLARE_FUNCTION(execGetRowClientInt)
		DECLARE_FUNCTION(execSetRowClientObject)
		DECLARE_FUNCTION(execGetRowClientObject)

		DECLARE_FUNCTION(execAddRow)
		DECLARE_FUNCTION(execDeleteRow)
		DECLARE_FUNCTION(execModifyRow)
		DECLARE_FUNCTION(execDeleteAllRows)
		DECLARE_FUNCTION(execSetField)
		DECLARE_FUNCTION(execGetField)
		DECLARE_FUNCTION(execSetFieldValue)
		DECLARE_FUNCTION(execGetFieldValue)

		DECLARE_FUNCTION(execGetNumRows)
		DECLARE_FUNCTION(execGetNumSelectedRows)
		DECLARE_FUNCTION(execSelectRow)
		DECLARE_FUNCTION(execSelectAllRows)
		DECLARE_FUNCTION(execSelectToRow)
		DECLARE_FUNCTION(execToggleRowSelection)
		DECLARE_FUNCTION(execIsRowSelected)
		DECLARE_FUNCTION(execGetSelectedRow)

		DECLARE_FUNCTION(execMoveRow)
		DECLARE_FUNCTION(execSetRow)

		DECLARE_FUNCTION(execSetFocusRow)
		DECLARE_FUNCTION(execGetFocusRow)

		DECLARE_FUNCTION(execSetNumColumns)
		DECLARE_FUNCTION(execGetNumColumns)
		DECLARE_FUNCTION(execResizeColumns)

		DECLARE_FUNCTION(execSetColumnTitle)
		DECLARE_FUNCTION(execGetColumnTitle)
		DECLARE_FUNCTION(execSetColumnWidth)
		DECLARE_FUNCTION(execGetColumnWidth)
		DECLARE_FUNCTION(execSetColumnAlignment)
		DECLARE_FUNCTION(execGetColumnAlignment)
		DECLARE_FUNCTION(execSetColumnColor)
		DECLARE_FUNCTION(execGetColumnColor)
		DECLARE_FUNCTION(execSetColumnFont)
		DECLARE_FUNCTION(execGetColumnFont)
		DECLARE_FUNCTION(execSetColumnType)
		DECLARE_FUNCTION(execGetColumnType)
		DECLARE_FUNCTION(execHideColumn)
		DECLARE_FUNCTION(execIsColumnHidden)

		DECLARE_FUNCTION(execSetSortColumn)
		DECLARE_FUNCTION(execAddSortColumn)
		DECLARE_FUNCTION(execRemoveSortColumn)
		DECLARE_FUNCTION(execResetSortColumns)
		DECLARE_FUNCTION(execSort)

		DECLARE_FUNCTION(execEnableHotKeys)
		DECLARE_FUNCTION(execSetHotKeyColumn)

		DECLARE_FUNCTION(execEnableAutoSort)
		DECLARE_FUNCTION(execIsAutoSortEnabled)
		DECLARE_FUNCTION(execEnableAutoExpandColumns)
		DECLARE_FUNCTION(execIsAutoExpandColumnsEnabled)
		DECLARE_FUNCTION(execEnableMultiSelect)
		DECLARE_FUNCTION(execIsMultiSelectEnabled)

		DECLARE_FUNCTION(execSetFieldMargins)
		DECLARE_FUNCTION(execGetFieldMargins)
		DECLARE_FUNCTION(execGetPageSize)
		DECLARE_FUNCTION(execSetDelimiter)
		DECLARE_FUNCTION(execSetHighlightTextColor)
		DECLARE_FUNCTION(execSetHighlightTexture)
		DECLARE_FUNCTION(execSetHighlightColor)
		DECLARE_FUNCTION(execSetFocusTexture)
		DECLARE_FUNCTION(execSetFocusColor)
		DECLARE_FUNCTION(execSetFocusThickness)

		DECLARE_FUNCTION(execShowFocusRow)

		DECLARE_FUNCTION(execSetListSounds)
		DECLARE_FUNCTION(execPlayListSound)

};  // XListWindow


#endif // _EXT_LIST_H_
