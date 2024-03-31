/*============================================================================
	UnUnix.h: Unix system-specific declarations.

	Revision history:
		* Created by Mike Danylchuk
		* Modified for Unreal Engine standards compliance by Brandon Reinhart
============================================================================*/

// Pathnames.
#define PATH(s) appUnixPath( s )
char* appUnixPath( const char* Path );

// Networking.
unsigned long appGetLocalIP( void );

// String functions.
int stricmp( const char* s, const char* t );
int strnicmp( const char* s, const char* t, int n );
char* strupr( char* s );

// Globally Unique Identifiers.
void appGetGUID( void* GUID );

// Signal Handling
void HandleSignal( int Signal );
void HandleInterrupt( int Signal );

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/

