/*=============================================================================
	AInternetLink.h.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	AInternetLink();
	void Destroy();
	UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );	
	SOCKET& GetSocket() 
	{ 
		return *(SOCKET*)&Socket;
	}
	FResolveInfo*& GetResolveInfo()
	{
		return *(FResolveInfo**)&PrivateResolveInfo;
	}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
