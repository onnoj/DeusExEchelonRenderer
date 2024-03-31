
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtCheckbox.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal checkbox widgets
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_CHECKBOX_H_
#define _EXT_CHECKBOX_H_


// ----------------------------------------------------------------------
// XCheckboxWindow class

class EXTENSION_API XCheckboxWindow : public XToggleWindow
{
	DECLARE_CLASS(XCheckboxWindow, XToggleWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XCheckboxWindow)

	public:
		XCheckboxWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);

		UTexture *toggleOff;
		UTexture *toggleOn;

		FLOAT    textureWidth;
		FLOAT    textureHeight;

		FLOAT    checkboxSpacing;
		BITFIELD bRightSide:1 GCC_PACK(4);

		BYTE     checkboxStyle GCC_PACK(4);
		FColor   checkboxColor;

	public:
		// XCheckboxWindow interface
		void SetCheckboxTextures(UTexture *newToggleOff=NULL, UTexture *newToggleOn=NULL,
		                         FLOAT newTextureWidth=0, FLOAT newTextureHeight=0);
		void SetCheckboxSpacing(FLOAT newSpacing);
		void ShowCheckboxOnRightSide(UBOOL bRight=TRUE)  { bRightSide=bRight; }
		void SetCheckboxStyle(BYTE newStyle);
		void SetCheckboxColor(FColor newColor);

		// XWindow interface callbacks
		void ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                  UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void Draw(XGC *gc);

	private:
		void ComputeTextureSize(FLOAT &tWidth,
		                        FLOAT &tHeight,
		                        FLOAT &tOffset);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetCheckboxTextures)
		DECLARE_FUNCTION(execSetCheckboxSpacing)
		DECLARE_FUNCTION(execShowCheckboxOnRightSide)
		DECLARE_FUNCTION(execSetCheckboxStyle)
		DECLARE_FUNCTION(execSetCheckboxColor)

};  // XCheckboxWindow


#endif // _EXT_CHECKBOX_H_
