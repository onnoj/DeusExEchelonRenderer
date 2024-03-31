/*=============================================================================
	AGameReplicationInfo.h.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
	// Constructors.
	AGameReplicationInfo() {}

	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map );

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
