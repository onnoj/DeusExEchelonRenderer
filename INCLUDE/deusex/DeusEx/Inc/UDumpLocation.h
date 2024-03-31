// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  DumpLocation.h
//  Programmer  :  Albert Yarusso
//  Description :  Header file for Dumping/Loading the player's location
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#include <stdio.h>

#ifndef _DEUSEX_DUMPLOCATION_H_
#define _DEUSEX_DUMPLOCATION_H_

#define DUMPLOCATION_EXTENSION TEXT("DMP")
#define DUMPLOCATION_HEADER TEXT("Deus Ex Dump Location File")

// ----------------------------------------------------------------------
// EDumpLocationStruct, this is how the information is represented
// on disk.

struct DumpLocationFileStruct
{
	bool bDeleted;
	INT LocationID;
	TCHAR MapName[64];
	FVector Location;
	FRotator ViewRotation;
	TCHAR GameVersion[64];
	TCHAR Title[128];
	TCHAR Desc[2048];
};

// Unreal-usable structure
struct DumpLocationStruct
{
	BITFIELD bDeleted:1;
	INT LocationID;
	FString MapName;
	FVector Location;
	FRotator ViewRotation;
	FString GameVersion;
	FString Title;
	FString Desc;
};

// ----------------------------------------------------------------------
// UDumpLocation class

class DEUSEX_API UDumpLocation: public UObject
{
	DECLARE_CLASS(UDumpLocation, UObject, 0)

	public:
		UDumpLocation();
		void Destroy(void);

		DumpLocationFileStruct *currentDumpFileLocation;
		DumpLocationStruct     currentDumpLocation;		

		void PopulateDumpDirectory(void);
		FString GetFirstDumpFile(void);
		FString GetNextDumpFile(void);
		INT  GetDumpFileIndex(void);
		INT  GetDumpFileCount(void);
		bool OpenDumpFile(FString &filename);
		bool CreateDumpFile(FString &filename);
		void CloseDumpFile(void);
		void DeleteDumpFile(FString &filename);
		bool GetFirstDumpFileLocation(void);
		bool GetNextDumpFileLocation(void);
		INT  GetDumpLocationIndex(void);
		bool SelectDumpFileLocation(INT dumpLocationID);
		void GetDumpFileLocationInfo();
		void DeleteDumpFileLocation(INT dumpLocationID);
		void AddDumpFileLocation(FString &filename, FString &newTitle, FString &newDescription);
		INT  GetNextDumpFileLocationID(void);
		FString GetCurrentUser(void);
		void SetPlayer(ADeusExPlayer *newPlayer);
		void SaveLocation(void);
		void LoadLocation(void);
		UBOOL HasLocationBeenSaved(void);
		INT  GetDumpFileLocationCount(FString &filename);

	private:
		TArray<FString> dumpFileDirectory;				
		INT currentDumpFileIndex;
		INT currentDumpLocationIndex;
		FString currentUser;
		FILE *dumpFile;
		INT dumpLocationCount;
		ADeusExPlayer *player;

		void SeekPastHeader(void);

	public:
		DECLARE_FUNCTION(execGetFirstDumpFile)
		DECLARE_FUNCTION(execGetNextDumpFile)
		DECLARE_FUNCTION(execGetDumpFileIndex)
		DECLARE_FUNCTION(execGetDumpFileCount);
		DECLARE_FUNCTION(execOpenDumpFile)
		DECLARE_FUNCTION(execCloseDumpFile)
		DECLARE_FUNCTION(execDeleteDumpFile)
		DECLARE_FUNCTION(execGetFirstDumpFileLocation)
		DECLARE_FUNCTION(execGetNextDumpFileLocation)
		DECLARE_FUNCTION(execGetDumpLocationIndex)
		DECLARE_FUNCTION(execSelectDumpFileLocation)
		DECLARE_FUNCTION(execGetDumpFileLocationInfo)
		DECLARE_FUNCTION(execDeleteDumpFileLocation)
		DECLARE_FUNCTION(execAddDumpFileLocation)
		DECLARE_FUNCTION(execGetNextDumpFileLocationID)
		DECLARE_FUNCTION(execGetCurrentUser)
		DECLARE_FUNCTION(execSetPlayer)
		DECLARE_FUNCTION(execSaveLocation)
		DECLARE_FUNCTION(execLoadLocation)
		DECLARE_FUNCTION(execHasLocationBeenSaved)
		DECLARE_FUNCTION(execGetDumpFileLocationCount)

};  // UDumpLocation

#endif // _DEUSEX_DUMPLOCATION_H_
