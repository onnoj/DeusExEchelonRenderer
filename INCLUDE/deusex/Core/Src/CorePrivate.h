/*=============================================================================
	CorePrivate.h: Unreal core private header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*----------------------------------------------------------------------------
	Core public includes.
----------------------------------------------------------------------------*/

#include "Core.h"

/*-----------------------------------------------------------------------------
	Locals functions.
-----------------------------------------------------------------------------*/

extern void appPlatformPreInit();
extern void appPlatformInit();
extern void appPlatformPreExit();
extern void appPlatformExit();

extern UBOOL GNoGC;
extern UBOOL GCheckConflicts;
extern UBOOL GExitPurge;

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/

#include "UnLinker.h"

/*-----------------------------------------------------------------------------
	UTextBufferFactory.
-----------------------------------------------------------------------------*/

//
// Imports UTextBuffer objects.
//
class CORE_API UTextBufferFactory : public UFactory
{
	DECLARE_CLASS(UTextBufferFactory,UFactory,0)

	// Constructors.
	UTextBufferFactory();
	void StaticConstructor();

	// UFactory interface.
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
