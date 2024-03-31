// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtGameDirecotyr.h
//  Programmer  :  Albert Yarusso
//  Description :  Header file for getting Unreal directory lists
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_GAMEDIRECTORY_H_
#define _EXT_GAMEDIRECTORY_H_

// ----------------------------------------------------------------------
// EGameDirectoryType - Game Directory Types

enum EGameDirectoryTypes
{
	GD_Maps			= 0,
	GD_SaveGames	= 1
};

// ----------------------------------------------------------------------
// XGameDirectory class

class DEUSEX_API XGameDirectory: public UObject
{
	DECLARE_CLASS(XGameDirectory, UObject, 0)

	public:
		XGameDirectory();
		void Destroy(void);
		void Serialize( FArchive& Ar );

		EGameDirectoryTypes gameDirectoryType;		// Game Directory Type
		FString currentFilter;						// Filter for screening files
		TArray<FString> directoryList;				// File listing for the current directory
		TArray<UDeusExSaveInfo*> loadedSaveInfoPointers;
		UDeusExSaveInfo *tempSaveInfo;

		void GetGameDirectory( void );
		void GetMapsDirectory( void );
		void GetSaveGamesDirectory( void );
		INT  GetDirCount( void );
		INT  GetNewSaveFileIndex( void );
		FString GenerateSaveFilename( INT saveIndex );
		FString GenerateNewSaveFilename( INT newIndex = -1 );
		const TCHAR *GetDirFilename( INT fileIndex );
		void SetDirType( EGameDirectoryTypes newGameDirectoryType );
		void SetDirFilter( FString &newFilter );
		UDeusExSaveInfo *GetSaveInfo(INT fileIndex);
		UDeusExSaveInfo *GetSaveInfoFromDirectoryIndex(INT directoryIndex);
		UDeusExSaveInfo *GetTempSaveInfo(void);
		INT GetSaveFreeSpace(void);
		INT GetSaveDirectorySize(INT saveIndex);
		INT GetGameIndex(void);
	
		void DeleteSaveInfo( UDeusExSaveInfo *saveInfo );
		void PurgeAllSaveInfo( void );

	public:
		DECLARE_FUNCTION(execGetGameDirectory)
		DECLARE_FUNCTION(execGetNewSaveFileIndex)
		DECLARE_FUNCTION(execGenerateSaveFilename)
		DECLARE_FUNCTION(execGenerateNewSaveFilename)
		DECLARE_FUNCTION(execGetDirCount)
		DECLARE_FUNCTION(execGetDirFilename)
		DECLARE_FUNCTION(execSetDirType)
		DECLARE_FUNCTION(execSetDirFilter)
		DECLARE_FUNCTION(execGetSaveInfo)
		DECLARE_FUNCTION(execGetTempSaveInfo)
		DECLARE_FUNCTION(execDeleteSaveInfo)
		DECLARE_FUNCTION(execPurgeAllSaveInfo)
		DECLARE_FUNCTION(execGetSaveFreeSpace)
		DECLARE_FUNCTION(execGetSaveDirectorySize)
		DECLARE_FUNCTION(execGetSaveInfoFromDirectoryIndex)

};  // XGameDirectory

#endif // _EXT_GAMEDIRECTORY_H_
