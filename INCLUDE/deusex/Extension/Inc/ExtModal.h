
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtModal.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal Modal window extension
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_MODAL_H_
#define _EXT_MODAL_H_


// ----------------------------------------------------------------------
// XModalWindow class

class EXTENSION_API XModalWindow : public XTabGroupWindow
{
	DECLARE_CLASS(XModalWindow, XTabGroupWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XModalWindow)

	friend class XWindow;
	friend class XTabGroupWindow;
	friend class XRootWindow;

	public:
		XModalWindow(XRootWindow *parent);

		// Structors
		void Init(XWindow *newParent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

		// Mouse focus mode
		BYTE    focusMode;

	private:
		XWindow *preferredFocus;

		// Array of tab group descendants, sorted by location
		TArray<XTabGroupWindow *> tabGroupWindowList;

		XWindow  *acceleratorTable[IK_MAX];
		BITFIELD bDirtyAccelerators:1 GCC_PACK(4);

	public:
		// XModalWindow interface
		void SetMouseFocusMode(EMouseFocusMode newFocusMode)
		{
			focusMode = newFocusMode;
		}

		UBOOL IsCurrentModal(void);

		void AddTabGroupToTable(XTabGroupWindow *tabGroup);
		void RemoveTabGroupFromTable(XTabGroupWindow *tabGroup);
		void ResortTabGroupTable(void);

		void    ClearAcceleratorTable(void);
		void    BuildAcceleratorTable(void);
		XWindow *GetAcceleratorWindow(TCHAR key);

		// XWindow interface callbacks
		void  VisibilityChanged(UBOOL bNewVisibility);
		void  DescendantRemoved(XWindow *pDescendant);
		void  Tick(FLOAT deltaSeconds);
		UBOOL KeyPressed(TCHAR key);

	private:
		void SetAcceleratorWindows(XWindow *pWindow);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetMouseFocusMode)
		DECLARE_FUNCTION(execIsCurrentModal)

};  // XModalWindow


#endif // _EXT_MODAL_H_
