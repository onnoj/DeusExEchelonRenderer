
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtTabGroup.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal Tab Group window extension
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_TAB_GROUP_H_
#define _EXT_TAB_GROUP_H_


// ----------------------------------------------------------------------
// XTabGroupWindow class

class EXTENSION_API XTabGroupWindow : public XWindow
{
	DECLARE_CLASS(XTabGroupWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XTabGroupWindow)

	public:
		XTabGroupWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

		// Sorted lists of selectable children
		TArray<XWindow *> rowMajorWindowList;
		TArray<XWindow *> colMajorWindowList;

		// Resizing options
		BITFIELD bSizeParentToChildren:1 GCC_PACK(4);
		BITFIELD bSizeChildrenToParent:1;

		// Option to enable wrapping when moving focus
		BITFIELD bWrapFocus:1;

		// Index into our modal window's list of tab groups
		INT tabGroupIndex GCC_PACK(4);

		// Absolute position of first child in tab group
		FLOAT firstAbsX;
		FLOAT firstAbsY;

	public:
		// XTabGroupWindow interface
		void  AddWindowToTables(XWindow *pWindow);
		void  RemoveWindowFromTables(XWindow *pWindow);
		void  ResortWindowTables(void);
		UBOOL ComputeTabGroupLocation(void);

		// XWindow interface callbacks
		void  ConfigurationChanged(void);
		void  ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                   UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void  ChildRequestedVisibilityChange(XWindow *pChild, UBOOL bNewVisibility);
		UBOOL ChildRequestedReconfiguration(XWindow *pChild);

};  // XTabGroupWindow


#endif // _EXT_TAB_GROUP_H_
