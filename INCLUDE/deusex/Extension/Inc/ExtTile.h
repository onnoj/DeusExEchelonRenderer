// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtTile.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal tiling widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_TILE_H_
#define _EXT_TILE_H_


// ----------------------------------------------------------------------
// XRowStruct - Private structure which holds information on one tile row
//              (NOTE: In tiles, a "row" is not necessarily horizontal;
//              its actual direction depends on the TileWindow's
//              orientation)

struct XRowStruct
{
	INT rowHeight;    // Size at right angles to orientation
	INT rowLength;    // Size along orientation axis
	INT rowItems;     // Number of children on this row
};


// ----------------------------------------------------------------------
// XTileWindow class

class EXTENSION_API XTileWindow : public XWindow
{
	DECLARE_CLASS(XTileWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XTileWindow)

	public:
		XTileWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		// Order information
		BYTE               orientation;    // Horizontal or vertical
		BYTE               hDirection;     // Left-right or right-left
		BYTE               vDirection;     // Top-bottom or bottom-top

		// Margins
		FLOAT              hMargin;        // Horizontal margin
		FLOAT              vMargin;        // Vertical margin

		// Spacing
		FLOAT              minorSpacing;   // Child spacing along orientation
		FLOAT              majorSpacing;   // Child spacing when wrapped

		// Child alignment relative to other children in the same row
		BYTE               hChildAlign;    // Horizontal child alignment
		BYTE               vChildAlign;    // Vertical child alignment

		// Wrapping
		BITFIELD           bWrap:1 GCC_PACK(4); // Wrap windows if not enough space?

		// Parent filling
		BITFIELD           bFillParent:1;  // Fill the parent with children?

		// Child sizing
		BITFIELD           bEqualWidth:1;  // Make all children equal width?
		BITFIELD           bEqualHeight:1; // Make all children equal height?

	private:
		TArray<XRowStruct> rowArray GCC_PACK(4);  // Temporary array of row information

	public:
		// XTileWindow interface
		void SetMargins(FLOAT newHMargin, FLOAT newVMargin)
		{
			hMargin = newHMargin;
			vMargin = newVMargin;
			AskParentForReconfigure();
		}
		void SetOrientation(EOrientation newOrientation)
		{
			orientation = newOrientation;
			AskParentForReconfigure();
		}
		void SetDirections(EHDirection newHDir, EVDirection newVDir)
		{
			hDirection = newHDir;
			vDirection = newVDir;
			AskParentForReconfigure();
		}
		void SetOrder(EOrder newOrder);
		void SetMinorSpacing(FLOAT newSpacing)
		{
			minorSpacing = newSpacing;
			AskParentForReconfigure();
		}
		void SetMajorSpacing(FLOAT newSpacing)
		{
			majorSpacing = newSpacing;
			AskParentForReconfigure();
		}
		void SetChildAlignments(EHAlign newHAlign, EVAlign newVAlign)
		{
			hChildAlign = newHAlign;
			vChildAlign = newVAlign;
			AskParentForReconfigure();
		}
		void EnableWrapping(UBOOL bWrappingOn)
		{
			bWrap = bWrappingOn;
			AskParentForReconfigure();
		}
		void FillParent(UBOOL bNewFillParent)
		{
			bFillParent = bNewFillParent;
			AskParentForReconfigure();
		}
		void MakeWidthsEqual(UBOOL bEqual)
		{
			bEqualWidth = bEqual;
			AskParentForReconfigure();
		}
		void MakeHeightsEqual(UBOOL bEqual)
		{
			bEqualHeight = bEqual;
			AskParentForReconfigure();
		}

		// XWindow interface callbacks
		void  ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                   UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void  ParentRequestedGranularity(FLOAT &hGranularity, FLOAT &vGranularity);

		void  ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		UBOOL ChildRequestedReconfiguration(XWindow *pChild);
		void  ConfigurationChanged(void);

	private:
		// Internal routines
		void ComputeChildSizes(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                       UBOOL bHeightSpecified, FLOAT &preferredHeight);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetMargins)
		DECLARE_FUNCTION(execSetOrientation)
		DECLARE_FUNCTION(execSetDirections)
		DECLARE_FUNCTION(execSetOrder)
		DECLARE_FUNCTION(execSetMinorSpacing)
		DECLARE_FUNCTION(execSetMajorSpacing)
		DECLARE_FUNCTION(execSetChildAlignments)
		DECLARE_FUNCTION(execEnableWrapping)
		DECLARE_FUNCTION(execFillParent)
		DECLARE_FUNCTION(execMakeWidthsEqual)
		DECLARE_FUNCTION(execMakeHeightsEqual)

};  // XTileWindow


#endif // _EXT_TILE_H_
