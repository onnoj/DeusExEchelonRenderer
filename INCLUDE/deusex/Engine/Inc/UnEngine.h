/*=============================================================================
	UnEngine.h: Unreal engine definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Unreal engine.
-----------------------------------------------------------------------------*/

class ENGINE_API UEngine : public USubsystem
{
	DECLARE_ABSTRACT_CLASS(UEngine,USubsystem,CLASS_Config|CLASS_Transient)

	// Subsystems.
	UClass*					GameRenderDeviceClass;
	UClass*					AudioDeviceClass;
	UClass*					ConsoleClass;
	UClass*					NetworkDriverClass;
	UClass*					LanguageClass;

	// Variables.
	class UPrimitive*		Cylinder;
	class UClient*			Client;
	class URenderBase*		Render;
	class UAudioSubsystem*	Audio;
	INT						TickCycles, GameCycles, ClientCycles;
	INT						CacheSizeMegs;
	BITFIELD				UseSound;
	FLOAT					CurrentTickRate;

	// Constructors.
	UEngine();
	void StaticConstructor();

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

#if !DEMOVERSION
	virtual void vtPad1() {}
	virtual void vtPad2() {}
	virtual void vtPad3() {}
#endif

	// UEngine interface.
	virtual void Init();
	virtual void Exit();
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Out=*GLog );
	virtual void Flush( UBOOL AllowPrecache );
	virtual UBOOL Key( UViewport* Viewport, EInputKey Key );
	virtual UBOOL InputEvent( UViewport* Viewport, EInputKey iKey, EInputAction State, FLOAT Delta=0.0 );
	virtual void Tick( FLOAT DeltaSeconds )=0;
	virtual void Draw( UViewport* Viewport, UBOOL Blit=1, BYTE* HitData=NULL, INT* HitSize=NULL )=0;
	virtual void MouseDelta( UViewport* Viewport, DWORD Buttons, FLOAT DX, FLOAT DY )=0;
	virtual void MousePosition( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y )=0;
	virtual void Click( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y )=0;
	virtual void SetClientTravel( UPlayer* Viewport, const TCHAR* NextURL, UBOOL bItems, ETravelType TravelType )=0;
	virtual INT ChallengeResponse( INT Challenge );
	virtual FLOAT GetMaxTickRate();
	virtual void SetProgress( const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds );
	void InitAudio();

	// Temporary!!
	virtual int edcamMode( UViewport* Viewport ) {return 0;}

#if !DEMOVERSION
	virtual void vtPad4() {}
	virtual void vtPad5() {}
	virtual void vtPad6() {}
#endif
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
