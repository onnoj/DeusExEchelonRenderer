/*=============================================================================
	UnPlayer.h: Unreal player class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPlayer.
-----------------------------------------------------------------------------*/

//
// A player, the base class of UViewport (local players) and UNetConnection (remote players).
//
class ENGINE_API UPlayer : public UObject, public FOutputDevice, public FExec
{
	DECLARE_ABSTRACT_CLASS(UPlayer,UObject,CLASS_Transient|CLASS_Config)

	// Objects.
	APlayerPawn*	Actor;
	UConsole*		Console;
	
	BITFIELD		bWindowsMouseAvailable:1;
	BITFIELD		bShowWindowsMouse:1;
	BITFIELD		bSuspendPrecaching:1;
	FLOAT			WindowsMouseX;
	FLOAT			WindowsMouseY;
	INT				CurrentNetSpeed, ConfiguredInternetSpeed, ConfiguredLanSpeed;
   FLOAT       StaticUpdateInterval, DynamicUpdateInterval;
	BYTE			SelectedCursor;

	// Constructor.
	UPlayer();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// FExec interface.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

	// UPlayer interface.
	virtual void ReadInput( FLOAT DeltaSeconds )=0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
