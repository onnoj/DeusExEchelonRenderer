/*============================================================================
	UnSocket.h: Common interface for WinSock and BSD sockets.

	Revision history:
		* Created by Mike Danylchuk
============================================================================*/

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

#if __WINSOCK__
	typedef INT					__SIZE_T__;
	#define GCC_OPT_INT_CAST
#endif

// Provide WinSock definitions for BSD sockets.
#if __BSD_SOCKETS__
	typedef int					SOCKET;
	typedef struct hostent		HOSTENT;
	typedef in_addr				IN_ADDR;
	typedef struct sockaddr		SOCKADDR;
	typedef struct sockaddr_in	SOCKADDR_IN;
	typedef struct linger		LINGER;
	typedef struct timeval		TIMEVAL;
	typedef TCHAR*				LPSTR;
	typedef SIZE_T				__SIZE_T__;

	#define INVALID_SOCKET		-1
	#define SOCKET_ERROR		-1
	//#define WSAEWOULDBLOCK		EWOULDBLOCK
	#define WSAEWOULDBLOCK		EINPROGRESS
	#define WSAENOTSOCK			ENOTSOCK
	#define WSATRY_AGAIN		TRY_AGAIN
	#define WSAHOST_NOT_FOUND	HOST_NOT_FOUND
	#define WSANO_DATA			NO_ADDRESS
	#define LPSOCKADDR			sockaddr*

	#define closesocket			close
	#define ioctlsocket			ioctl
	#define WSAGetLastError()	errno

	#define GCC_OPT_INT_CAST	(DWORD*)
#endif

// IP address macros.
#if __WINSOCK__
	#define IP(sin_addr,n) sin_addr.S_un.S_un_b.s_b##n
#elif __BSD_SOCKETS__
	#define IP(sin_addr,n) ((BYTE*)&sin_addr.s_addr)[n-1]
#endif

/*----------------------------------------------------------------------------
	Functions.
----------------------------------------------------------------------------*/

UBOOL InitSockets( FString& Error );
TCHAR* SocketError( INT Code=-1 );
UBOOL IpMatches( sockaddr_in& A, sockaddr_in& B );
void IpGetBytes( in_addr Addr, BYTE& Ip1, BYTE& Ip2, BYTE& Ip3, BYTE& Ip4 );
void IpSetBytes( in_addr& Addr, BYTE Ip1, BYTE Ip2, BYTE Ip3, BYTE Ip4 );
void IpGetInt( in_addr Addr, DWORD& Ip );
void IpSetInt( in_addr& Addr, DWORD Ip );
FString IpString( in_addr Addr, INT Port=0 );

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
