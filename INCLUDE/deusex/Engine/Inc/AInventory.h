/*=============================================================================
	AInventory.h.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	// Constructors.
	AInventory() {}

	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map );
	virtual UBOOL ShouldDoScriptReplication();

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
