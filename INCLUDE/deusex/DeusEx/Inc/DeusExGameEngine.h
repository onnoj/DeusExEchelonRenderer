// ----------------------------------------------------------------------
//  File Name   :  DeusExGameEngine.h
//  Programmer  :  Albert Yarusso
//  Description :  Header for the DeusEx GameEngine 
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _DEUSEX_GAME_ENGINE_H_
#define _DEUSEX_GAME_ENGINE_H_

#define SAVEINFO_Name			TEXT("MyDeusExSaveInfo")
#define SAVE_CurrentDirectory	TEXT("Current")
#define SAVE_QuickSaveDirectory TEXT("QuickSave")
#define SAVE_SaveInfoFilename	TEXT("SaveInfo")
#define SAVE_DirPrefix			TEXT("Save")

// ----------------------------------------------------------------------
// DDeusExGameEngine class

class DEUSEX_API DDeusExGameEngine : public XGameEngineExt
{
	DECLARE_CLASS(DDeusExGameEngine, XGameEngineExt, CLASS_Config|CLASS_Transient)

	public:
		DDeusExGameEngine();

		void  DeleteGame(INT Position );
		UDeusExSaveInfo* GetSaveInfo(UPackage *saveInfoPackage);
		void CopySaveGameFiles(FString &source, FString &dest);
		void DeleteSaveGameFiles(FString saveDirectory = TEXT(""));
		UDeusExSaveInfo* LoadSaveInfo(int DirectoryIndex = -2);
		void PruneTravelActors(void);
		void SaveCurrentLevel(int DirectoryIndex = -2, bool bSavePlayer = FALSE);
		INT GetCurrentMissionNumber(void);
		INT GetNextMissionNumber(FString &mapName);
		ADeusExLevelInfo* GetDeusExLevelInfo(void);

		// Overridden
		void  Init(void);
		UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar=*GLog);
		void  SaveGame(INT Position, FString saveDesc = TEXT(""));
		UBOOL Browse( FURL URL, const TMap<FString,FString>* TravelInfo, FString& Error );

};  // DDeusExGameEngine


#endif // _DEUSEX_GAME_ENGINE_H_
