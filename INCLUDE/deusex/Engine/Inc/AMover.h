/*=============================================================================
	AMover.h: Class functions residing in the AMover class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	// Constructors.
	AMover();

	// UObject interface.
	void PostLoad();
	void PostEditChange();

	// AActor interface.
	void Spawned();
	void PostEditMove();
	void PreRaytrace();
	void PostRaytrace();
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map );

	// ABrush interface.
	virtual void SetWorldRaytraceKey();
	virtual void SetBrushRaytraceKey();

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
