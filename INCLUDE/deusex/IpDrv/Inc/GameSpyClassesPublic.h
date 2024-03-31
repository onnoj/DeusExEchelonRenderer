/*=============================================================================
	GameSpyClassesPublic.h: Public interface to private code for gamespy protocol.
	Copyright 1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Brandon Reinhart.
=============================================================================*/

typedef BYTE trip_t[3];
typedef BYTE kwart_t[4];

typedef struct FRC4Key
{      
	BYTE state[256];       
	BYTE x;        
	BYTE y;
} FRC4Key;

/*------------------------------*
 * GameSpy Validation Functions *
 *------------------------------*/

void GenerateSecretKey( BYTE* key, const TCHAR *GameName )
{
}

void gs_encrypt(BYTE *buffer_ptr, INT buffer_len, BYTE *key)
{
}

void gs_encode(BYTE *ins, INT size, BYTE *result)
{
}

void gs_decode(BYTE *ins, BYTE *result)
{
}

void rc4(BYTE *buffer_ptr, INT buffer_len, FRC4Key *key)
{
}
