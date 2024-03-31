// ----------------------------------------------------------------------
//  File Name   :  DEUSEXTEXT.h
//  Programmer  :  Albert Yarusso
//  Description :  Main header for the DeusExText Package and DLL
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _DEUSEXTEXT_H_
#define _DEUSEXTEXT_H_

#include "engine.h"

// ----------------------------------------------------------------------
// Needed for all routines and classes visible outside the DLL

#ifndef DEUSEXTEXT_API
#define DEUSEXTEXT_API DLL_IMPORT
#endif

// ----------------------------------------------------------------------
// Define boolean values, if not already defined

#ifndef TRUE
	#define TRUE (1)
#endif

#ifndef FALSE
	#define FALSE (0)
#endif
	
// ----------------------------------------------------------------------
// Enumerations used for intrinsic routines in the DEUSEXTEXT DLL

enum
{
	DEUSEXTEXT_GetText=2200,
	DEUSEXTEXT_TextParserOpenText=2210,
	DEUSEXTEXT_TextParserCloseText=2211,
	DEUSEXTEXT_TextParserProcessText=2212,
	DEUSEXTEXT_IsEOF=2213,
	DEUSEXTEXT_TextParserGetText=2214,
	DEUSEXTEXT_TextParserGotoLabel=2215,
	DEUSEXTEXT_TextParserGetTag=2216,
	DEUSEXTEXT_TextParserGetName=2217,
	DEUSEXTEXT_TextParserGetColor=2218,
	DEUSEXTEXT_TextParserGetEmailInfo=2219,
	DEUSEXTEXT_TextParserGetFileInfo=2220,
	DEUSEXTEXT_TextSetPlayerName=2221
};

#endif // _DEUSEXTEXT_H_

