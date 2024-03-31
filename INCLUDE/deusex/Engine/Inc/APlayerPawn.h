/*=============================================================================
	APlayerPawn.h: A player pawn.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map );

	// APlayerPawn interface.
	void SetPlayer( UPlayer* Player );

	// PlayerPawn Render Control Interface (RCI).
	// override the operations in the game-specific PlayerPawn class to control rendering
	virtual bool ClearScreen();
	//{ return false; };
	virtual bool RecomputeLighting();
	//{ return false; };
	virtual bool CanSee( const AActor* Actor );
	//{ return false };
	virtual INT GetViewZone( INT iViewZone, const UModel* Model );
	//{ return iViewZone; };
	virtual bool IsZoneVisible( INT iZone );
	//{ return true; }
	virtual bool IsSurfVisible( const FBspNode* Node, INT iZone, const FBspSurf* Poly );
	//{ return true; }
	virtual bool IsActorVisible( const AActor* Actor );
	//{ return true; }
   virtual bool AddAdditionalView( AActor* Actor );
   virtual void RemoveAdditionalView( const AActor* Actor ); 

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
