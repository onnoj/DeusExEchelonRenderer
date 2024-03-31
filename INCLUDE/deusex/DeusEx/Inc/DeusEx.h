// ----------------------------------------------------------------------
//  File Name   :  DeusEx.h
//  Programmer  :  Albert Yarusso
//  Description :  Main header for the DeusEx package and DLL
// ----------------------------------------------------------------------
//  Copyright ©1998 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#include "engine.h"
#include "extension.h"
#include "AAugmentationManager.h"
#include "UParticle.h" //DEUS_EX AMSD I suck.

// ----------------------------------------------------------------------
// Needed for all routines and classes visible outside the DLL

#ifndef DEUSEX_API
#define DEUSEX_API DLL_IMPORT
#endif

/*
// ----------------------------------------------------------------------
// Methods in UnrealScript that can be called from C++

#ifndef NAMES_ONLY
#define DECLARE_NAME(name) extern DEUSEX_API FName DEUSEX_##name;
#endif

DECLARE_NAME(ComputerStart)
DECLARE_NAME(ComputerInputFinished)

#ifndef NAMES_ONLY
#undef DECLARE_NAME
#endif
*/

#ifndef _DEUSEX_H_
#define _DEUSEX_H_

// ----------------------------------------------------------------------
// A global function

extern void DeusExInitNames(void);


// ----------------------------------------------------------------------
// Enumerations used for intrinsic routines in the ConSys DLL

enum
{
	DEUSEX_GetDeusExVersion=1099,

	DEUSEX_PlayerConBindEvents=2100,
	DEUSEX_DecorationConBindEvents=2101,
	DEUSEX_ScriptedPawnConBindEvents=2102,
	DEUSEX_SetBoolFlagFromString=3001,
	DEUSEX_CreateHistoryObject=3002,
	DEUSEX_CreateHistoryEvent=3003,
	DEUSEX_CreateLogObject=3010,
	DEUSEX_SaveGame=3011,
	DEUSEX_DeleteSaveGameFiles=3012,
	DEUSEX_CreateGameDirectoryObject=3013,
	DEUSEX_CreateDataVaultImageObject=3014,
	DEUSEX_CreateDumpLocationObject=3015,
	DEUSEX_UnloadTexture=3016,

	DEUSEX_DumpLoc_GetFirstDumpFile=3020,
	DEUSEX_DumpLoc_GetNextDumpFile=3021,
	DEUSEX_DumpLoc_GetDumpFileIndex=3022,
	DEUSEX_DumpLoc_GetDumpFileCount=3023,
	DEUSEX_DumpLoc_OpenDumpFile=3024,
	DEUSEX_DumpLoc_CloseDumpFile=3025,
	DEUSEX_DumpLoc_DeleteDumpFile=3026,
	DEUSEX_DumpLoc_GetFirstDumpFileLocation=3027,
	DEUSEX_DumpLoc_GetNextDumpFileLocation=3028,
	DEUSEX_DumpLoc_GetDumpLocationIndex=3029,
	DEUSEX_DumpLoc_SelectDumpFileLocation=3030,
	DEUSEX_DumpLoc_GetDumpFileLocationInfo=3031,
	DEUSEX_DumpLoc_DeleteDumpFileLocation=3032,
	DEUSEX_DumpLoc_AddDumpFileLocation=3033,
	DEUSEX_DumpLoc_GetNextDumpFileLocationID=3034,
	DEUSEX_DumpLoc_GetCurrentUser=3035,
	DEUSEX_DumpLoc_SetPlayer=3036,
	DEUSEX_DumpLoc_SaveLocation=3037,
	DEUSEX_DumpLoc_LoadLocation=3038,
	DEUSEX_DumpLoc_HasLocationBeenSaved=3039,
	DEUSEX_DumpLoc_GetDumpFileLocationCount=3040,

	DEUSEX_UpdateTimeStamp=3075,

	DEUSEX_GetGameDirectory=3080,
	DEUSEX_GetNewSaveFileIndex=3081,
	DEUSEX_GenerateSaveFilename=3082,
	DEUSEX_GenerateNewSaveFilename=3083,
	DEUSEX_GetDirCount=3084,
	DEUSEX_GetDirFilename=3085,
	DEUSEX_SetDirType=3086,
	DEUSEX_SetDirFilter=3087,
	DEUSEX_GetSaveInfo=3088,
	DEUSEX_GetSaveInfoFromDirectoryIndex=3089,
	DEUSEX_GetTempSaveInfo=3090,
	DEUSEX_DeleteSaveInfo=3091,
	DEUSEX_PurgeAllSaveInfo=3092,
	DEUSEX_GetSaveFreeSpace=3093,
    DEUSEX_GetSaveDirectorySize=3094
};

// ----------------------------------------------------------------------
// Structures required by native classes

struct FInitialAllianceInfo  {
	FName    AllianceName;
	FLOAT    AllianceLevel;
	BITFIELD bPermanent:1;  GCC_PACK(4);
};

struct FAllianceInfoEx  {
	FName    AllianceName;
	FLOAT    AllianceLevel;
	FLOAT    AgitationLevel;
	BITFIELD bPermanent:1;  GCC_PACK(4);
};

struct FInventoryItem  {
	class UClass *Inventory;
	INT          Count;
};

// ----------------------------------------------------------------------

#endif // _DEUSEX_H_

#include "DeusExClasses.h"

