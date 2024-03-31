/*=============================================================================
	ATcpLink.h.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	ATcpLink();
	void Destroy();
	UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );	

	void CheckConnectionAttempt();
	void CheckConnectionQueue();
	void PollConnections();
	UBOOL FlushSendBuffer();
	void ShutdownConnection();

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
