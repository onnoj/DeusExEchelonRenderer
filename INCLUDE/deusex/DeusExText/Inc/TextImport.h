// ----------------------------------------------------------------------
//  File Name   :  TextImport.h
//  Programmer  :  Albert Yarusso (ay)
//  Description :  DeusExText Import File Import Header
// ----------------------------------------------------------------------
//  Copyright ©1998 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _TEXTIMPORT_H_
#define _TEXTIMPORT_H_

#include "DeusExText.h"

void DLL_EXPORT __cdecl ImportAllDeusExTextFiles(FString packageName);
void DLL_EXPORT __cdecl ImportDeusExTextDirectory(FString packageName, FString importDirectory);
void DLL_EXPORT __cdecl ImportDeusExTextFile(FString packageName, FString importFileName);

// ----------------------------------------------------------------------
// DTextImport Class
// ----------------------------------------------------------------------

class DEUSEXTEXT_API DTextImport : public UObject
{
	DECLARE_CLASS(DTextImport, UObject, CLASS_Transient)

public:
	FILE *inFile;

	// Constructor
	DTextImport();
	void Destroy(void);

	// Import File
 	void ImportFile(FString packageName, FString importFileName);

	TCHAR* ConvertAnsiToUnicode( const ANSICHAR* ACh );
	FString ExtractBaseName(const TCHAR *fileName);
};

#endif // _TEXTIMPORT_H_
