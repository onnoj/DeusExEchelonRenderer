
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtObject.h
//  Programmer  :  Scott Martin
//  Description :  Common class for all Extension objects
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

// We need to have a common class for all Extension objects because
// UnrealScript doesn't allow one class to access the enumerations of
// another class.

// I believe I speak for all developers here when I make the following
// observation:  barf.


#ifndef _EXT_OBJECT_H_
#define _EXT_OBJECT_H_


// ----------------------------------------------------------------------
// Enumerations
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// EFlagType - Flag types

enum EFlagType
{
	FLAG_Bool,
	FLAG_Byte,
	FLAG_Int,
	FLAG_Float,
	FLAG_Name,
	FLAG_Vector,
	FLAG_Rotator,
	MAX_FLAG_TYPE
};


// ----------------------------------------------------------------------
// EHAlign - Horizontal alignments

enum EHAlign
{
	HALIGN_Left,
	HALIGN_Center,
	HALIGN_Right,
	HALIGN_Full
};


// ----------------------------------------------------------------------
// EVAlign - Vertical alignments

enum EVAlign
{
	VALIGN_Top,
	VALIGN_Center,
	VALIGN_Bottom,
	VALIGN_Full
};


// ----------------------------------------------------------------------
// EHDirection - Horizontal directions

enum EHDirection
{
	HDIR_LeftToRight,
	HDIR_RightToLeft
};


// ----------------------------------------------------------------------
// EVDirection - Vertical directions

enum EVDirection
{
	VDIR_TopToBottom,
	VDIR_BottomToTop
};


// ----------------------------------------------------------------------
// EOrder - Horizontal and vertical order

enum EOrder
{
	ORDER_Right,
	ORDER_Left,
	ORDER_Down,
	ORDER_Up,
	ORDER_RightThenDown,
	ORDER_RightThenUp,
	ORDER_LeftThenDown,
	ORDER_LeftThenUp,
	ORDER_DownThenRight,
	ORDER_DownThenLeft,
	ORDER_UpThenRight,
	ORDER_UpThenLeft
};


// ----------------------------------------------------------------------
// EMouseFocusMode - How mouse actions affect focus

enum EMouseFocusMode
{
	MFOCUS_None,
	MFOCUS_Click,
	MFOCUS_Enter,
	MFOCUS_EnterLeave
};


// ----------------------------------------------------------------------
// EMove - Movement direction

enum EMove
{
	MOVE_Left,
	MOVE_Right,
	MOVE_Up,
	MOVE_Down
};


// ----------------------------------------------------------------------
// EOrientation - Orientation (duh)

enum EOrientation
{
	ORIENT_Horizontal,
	ORIENT_Vertical
};


// ----------------------------------------------------------------------
// EMoveList - List window movement enumeration

enum EMoveList
{
	MOVELIST_Up,
	MOVELIST_Down,
	MOVELIST_PageUp,
	MOVELIST_PageDown,
	MOVELIST_Home,
	MOVELIST_End
};


// ----------------------------------------------------------------------
// EMoveInsert - Insertion point movement enumeration

enum EMoveInsert
{
	MOVEINSERT_Up,
	MOVEINSERT_Down,
	MOVEINSERT_Left,
	MOVEINSERT_Right,
	MOVEINSERT_WordLeft,
	MOVEINSERT_WordRight,
	MOVEINSERT_StartOfLine,
	MOVEINSERT_EndOfLine,
	MOVEINSERT_PageUp,
	MOVEINSERT_PageDown,
	MOVEINSERT_Home,
	MOVEINSERT_End
};


// ----------------------------------------------------------------------
// EInsertionPointType - Insertion point cursor type

enum EInsertionPointType
{
	INSTYPE_Insert,
	INSTYPE_Underscore,
	INSTYPE_Block,
	INSTYPE_RawInsert,
	INSTYPE_RawOverlay
};


// ----------------------------------------------------------------------
// EColumnType - Type of data contained in a column

enum EColumnType
{
	COLTYPE_String,
	COLTYPE_Float,
	COLTYPE_Time
};


// ----------------------------------------------------------------------
// EMoveThumb - Enumeration for thumb movement in a scrollbar

enum EMoveThumb
{
	MOVETHUMB_Home,
	MOVETHUMB_End,
	MOVETHUMB_Prev,
	MOVETHUMB_Next,
	MOVETHUMB_StepUp,
	MOVETHUMB_StepDown,
	MOVETHUMB_PageUp,
	MOVETHUMB_PageDown
};


// ----------------------------------------------------------------------
// XExtensionObject class

class EXTENSION_API XExtensionObject : public UObject
{
	DECLARE_CLASS(XExtensionObject, UObject, 0)

	public:
		XExtensionObject() : UObject() {}

		// Structors
		void Destroy(void)  { Super::Destroy(); }

		// Intrinsics
		DECLARE_FUNCTION(execStringToName)

};  // XExtensionObject


#endif // _EXT_OBJECT_H_
