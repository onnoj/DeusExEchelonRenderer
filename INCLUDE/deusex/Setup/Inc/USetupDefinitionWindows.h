/*=============================================================================
	USetupDefinitionWindows.h: Unreal Windows setup.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

/*-----------------------------------------------------------------------------
	USetupShortcut.
-----------------------------------------------------------------------------*/

class USetupShortcut : public USetupObject
{
	DECLARE_CLASS(USetupShortcut,USetupObject,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig);

	// Variables.
	FStringNoInit Template, WorkingDirectory, Command, Parameters, Icon; // Per install.
	FStringNoInit Caption; // Localized.

	// Functions.
	void StaticConstructor()
	{
		guard(USetupProduct::StaticConstructor);

		new(GetClass(),TEXT("Caption"         ),RF_Public)UStrProperty(CPP_PROPERTY(Caption         ), TEXT(""), CPF_Localized);
		new(GetClass(),TEXT("Template"        ),RF_Public)UStrProperty(CPP_PROPERTY(Template        ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("WorkingDirectory"),RF_Public)UStrProperty(CPP_PROPERTY(WorkingDirectory), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("Command"         ),RF_Public)UStrProperty(CPP_PROPERTY(Command         ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("Parameters"      ),RF_Public)UStrProperty(CPP_PROPERTY(Parameters      ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("Icon"            ),RF_Public)UStrProperty(CPP_PROPERTY(Icon            ), TEXT(""), CPF_Config   );

		unguard;
	}
};

/*-----------------------------------------------------------------------------
	USetupGroupWindows.
-----------------------------------------------------------------------------*/

class USetupGroupWindows : public USetupGroup
{
	DECLARE_CLASS(USetupGroupWindows,USetupGroup,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig)

	// Variables.
	FString DirectXHook;
	TArray<FString> WinRegistry, Shortcut;

	// Functions.
	void StaticConstructor()
	{
		guard(USetupProductWindows::StaticConstructor);
		UArrayProperty* P;

		  new(GetClass(),TEXT("DirectXHook"    ),RF_Public)UStrProperty  (CPP_PROPERTY(DirectXHook   ), TEXT(""), CPF_Config   );
		P=new(GetClass(),TEXT("WinRegistry"    ),RF_Public)UArrayProperty(CPP_PROPERTY(WinRegistry   ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Shortcut"       ),RF_Public)UArrayProperty(CPP_PROPERTY(Shortcut      ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;

		unguard;
	}
	USetupGroupWindows()
	: USetupGroup(), WinRegistry(E_NoInit), Shortcut(E_NoInit), DirectXHook(E_NoInit)
	{}
};

/*-----------------------------------------------------------------------------
	USetupDefinitionWindows.
-----------------------------------------------------------------------------*/

class USetupDefinitionWindows : public USetupDefinition
{
	DECLARE_CLASS(USetupDefinitionWindows,USetupDefinition,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig)

	// Windows related variables.
	FString WinPath;
	FString	WinSysPath;
	FString	DesktopPath;
	FString	ProgramsPath;
	FString	FavoritesPath;
	FString	StartupPath;
	FString	CommonProgramsPath;
	FString	CommonFavoritesPath;
	FString CommonStartupPath;
	HANDLE  hWndManager;

	// Functions.
	USetupDefinitionWindows();
	UBOOL GetRegisteredProductFolder( FString Product, FString& Folder );
	void SetupFormatStrings();
	void ProcessPostCopy( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	void ProcessExtra( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	void PreExit();
	void ProcessUninstallRemove( FString Key, FString Value, FInstallPoll* Poll );
	void PerformUninstallCopy();
	void CreateRootGroup();
	void DoInstallSteps( FInstallPoll *Poll );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
