
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtInput.h
//  Programmer  :  Scott Martin
//  Description :  Header for the Input extended class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_INPUT_H_
#define _EXT_INPUT_H_


// ----------------------------------------------------------------------
// XInputExt class

class EXTENSION_API XInputExt : public UInput
{
	DECLARE_CLASS(XInputExt, UInput, CLASS_Transient | CLASS_Config)

	public:
		XInputExt();

		// UInput interface
		UBOOL PreProcess(EInputKey iKey, EInputAction state, FLOAT delta=0.0) { return (TRUE); } // HACK HACK HACK!
		UBOOL Process(FOutputDevice &out, EInputKey iKey, EInputAction state, FLOAT delta=0.0);
		UBOOL Key(EInputKey iKey);  // MEGA-BOOGER!!

};  // XInputExt


#endif // _EXT_INPUT_H_
