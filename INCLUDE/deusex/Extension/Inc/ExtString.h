
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtString.h
//  Programmer  :  Albert Yarusso
//  Description :  Augmented strings
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_STRING_H_
#define _EXT_STRING_H_

#define TEXT_PART_SIZE		239

// ----------------------------------------------------------------------
// XExtString class

class EXTENSION_API XExtString : public UObject
{
	DECLARE_CLASS(XExtString, UObject, 0)
//	NO_DEFAULT_CONSTRUCTOR(XExtString)

	protected:
		INT  speechPage;							// Used for GetFirst/Next functions
		FStringNoInit text;

	public:
		// XExtString interface
		XExtString();
		void Serialize( FArchive& Ar );
		virtual void SetText(const TCHAR *newText);
		virtual void AppendText(const TCHAR *newText);
		const TCHAR *GetText(void)  { return (*text); }

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetText)
		DECLARE_FUNCTION(execAppendText)
		DECLARE_FUNCTION(execGetText)
		DECLARE_FUNCTION(execGetTextLength)
		DECLARE_FUNCTION(execGetTextPart)
		DECLARE_FUNCTION(execGetFirstTextPart)
		DECLARE_FUNCTION(execGetNextTextPart)

};  // XExtString


#endif // _EXT_STRING_H_
