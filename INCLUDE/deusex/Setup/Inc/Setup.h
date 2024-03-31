/*=============================================================================
	Filer.h: Unreal Filer public definitions.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Classes.
class	USetupObject;
class       USetupGroup;
class		USetupProduct;
class			USetupDefinition;
class				USetupDefinitionWindows;

// Constants.
#define PER_FILE_OVERHEAD    (65536+16384)
#define PER_INSTALL_OVERHEAD (16*1024*1024)

// Interfaces..
extern class FInstallPoll
{
public:
	virtual UBOOL Poll( const TCHAR* Label, SQWORD LocalBytes, SQWORD LocalTotal, SQWORD RunningBytes, SQWORD TotalBytes )
	{
		return 0;
	}
} GNoPoll;

// Types.
typedef void (USetupDefinition::*INSTALL_STEP)(FString,FString,UBOOL,FInstallPoll*);

// Manifest.
#define MANIFEST_FILE TEXT("Manifest")
#define MANIFEST_EXT  TEXT(".ini")
#define SFX_STUB      TEXT("RunSFX.exe")
#define MOD_EXT       TEXT(".umod")
#define SETUP_INI     MANIFEST_FILE MANIFEST_EXT
#define MOD_MAGIC     0x9fe3c5a3

// Archives.
enum {ARCHIVE_MAGIC=0x9fe3c5a3};
enum {ARCHIVE_HEADER_SIZE=5*4};
enum {ARCHIVE_VERSION=1};
enum EArchiveFlags
{
	ARCHIVEF_Bootstrap      = 0x00000001,
	ARCHIVEF_Unbootstrap    = 0x00000002,
	ARCHIVEF_Compressed     = 0x00000004,
};
struct FArchiveItem
{
	FString _Filename_;
	DWORD   Offset;
	DWORD   Size;
	DWORD	Flags;
	FArchiveItem()
	{}
	FArchiveItem( const TCHAR* InFilename, DWORD InOffset, DWORD InSize, DWORD InFlags )
	: _Filename_(InFilename), Offset(InOffset), Size(InSize), Flags(InFlags)
	{}
	friend FArchive& operator<<( FArchive& Ar, FArchiveItem& Item )
	{
		guard(FArchiveItem<<);
		return Ar << Item._Filename_ << Item.Offset << Item.Size << Item.Flags;
		unguard;
	}
};
struct FArchiveHeader
{
	INT Magic, TableOffset, FileSize, Ver, CRC;
	TArray<FArchiveItem> _Items_;
	FArchiveHeader()
	: Magic(MOD_MAGIC), TableOffset(-1), FileSize(0), Ver(ARCHIVE_VERSION), CRC(0)
	{}
	friend FArchive& operator<<( FArchive& Ar, FArchiveHeader& Head )
	{
		guard(FArchiveHeader<<);
		// Must match ARCHIVE_HEADER_SIZE.
		return Ar << Head.Magic << Head.TableOffset << Head.FileSize << Head.Ver << Head.CRC;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	Functions.
-----------------------------------------------------------------------------*/

//
// Return the name part only.
//
inline FString BaseFilename( FString Src )
{
	guard(BaseFilename);
	for( ; ; )
	{
		INT i = Src.InStr( PATH_SEPARATOR ); 
		if( i<0 )
			break;
		Src = Src.Mid(i+1);
	}
	return Src;
	unguard;
}

//
// Return the path part only.
//
inline FString BasePath( const FString& Src )
{
	guard(BasePath);
	for( INT i=Src.Len()-1; i>=0; i-- )
		if( Src.Mid(i,1)==PATH_SEPARATOR )
			return Src.Left(i);
	return TEXT("");
	unguard;
}

//
// Recurive file find.
//
inline TArray<FString> FindFilesRecursive( FString Path, FString Spec )
{
	guard(FindFilesRecursive);
	TArray<FString> Result = GFileManager->FindFiles( *(Path * Spec), 1, 0 );
	TArray<FString> Dirs   = GFileManager->FindFiles( *(Path * TEXT("*")), 0, 1 );
	for( TArray<FString>::TIterator It(Dirs); It; ++It )
	{
		TArray<FString> SubFiles = FindFilesRecursive( *(Path*(*It)), Spec );
		for( TArray<FString>::TIterator Jt(SubFiles); Jt; ++Jt )
			new(Result)FString( (*It) * (*Jt) );
	}
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	File information.
-----------------------------------------------------------------------------*/

//
// Information about a file.
//
struct FFileInfo
{
	FString Dest, Src, Master, Ref, Lang;
	SQWORD  Size, RefSize;
	UBOOL   MasterRecurse;
	DWORD   Flags;
	FFileInfo( FString Init )
	: Dest(), Src(), Ref(), Lang(), Size(0), RefSize(0), MasterRecurse(0), Flags(0)
	{
		guard(FFileInfo::FFileInfo);
		if( Init.Left(1)==TEXT("(") )
			Init = Init.Mid(1);
		if( Init.Right(1)==TEXT(")") )
			Init = Init.LeftChop(1);
		Parse( *Init, TEXT("DEST="),   Dest   );
		Parse( *Init, TEXT("SRC="),    Src    );
		Parse( *Init, TEXT("MASTER="), Master );
		Parse( *Init, TEXT("REF="),    Ref    );
		Parse( *Init, TEXT("REFSIZE="),RefSize);
		Parse( *Init, TEXT("SIZE="),   Size   );
		Parse( *Init, TEXT("LANG="),   Lang   );
		Parse( *Init, TEXT("FLAGS="),  Flags  );
		ParseUBOOL( *Init, TEXT("MASTERRECURSE="), MasterRecurse );
		unguard;
	}
	FString Safe( const FString& Str )
	{
		if( Src.InStr(TEXT(" "))>=0 )
			return US+TEXT("\"") + Str + TEXT("\"");
		return Str;
	}
	void Write( FOutputDevice& Ar, UBOOL Distribution )
	{
		guard(FFileInfo::Write);
		Ar.Logf( TEXT("(Src=%s"), *Safe(Src) );
		if( Dest!=TEXT("") )
			Ar.Logf( TEXT(",Dest=%s"), *Safe(Dest) );
		if( Distribution && Master!=TEXT("") )
			Ar.Logf( TEXT(",Master=%s"), *Safe(Master) );
		if( Ref!=TEXT("") )
			Ar.Logf( TEXT(",Ref=%s"), *Safe(Ref) );
		if( RefSize )
			Ar.Logf( TEXT(",RefSize=%i"), RefSize );
		if( Lang!=TEXT("") )
			Ar.Logf( TEXT(",Lang=%s"), *Lang );
		if( Size!=0 )
			Ar.Logf( TEXT(",Size=%i"), Size );
		if( Flags!=0 )
			Ar.Logf( TEXT(",Flags=%i"), Flags );
		if( Distribution && MasterRecurse )
			Ar.Logf( TEXT(",MasterRecurse=True"), Size );
		Ar.Log( TEXT(")") );
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	USetupObject.
-----------------------------------------------------------------------------*/

class USetupObject : public UObject
{
	DECLARE_ABSTRACT_CLASS(USetupObject,UObject,0)
};

/*-----------------------------------------------------------------------------
	USetupProduct.
-----------------------------------------------------------------------------*/

class USetupProduct : public USetupObject
{
	DECLARE_CLASS(USetupProduct,USetupObject,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig)

	// Data variables.
	FStringNoInit Product, Version; // Per install.
	FStringNoInit OldVersionInstallCheck, OldVersionNumber; //oldver Per install.
	FStringNoInit LocalProduct, Developer, ProductURL, VersionURL, DeveloperURL; // Localized.

	// Functions.
	void StaticConstructor()
	{
		guard(USetupProduct::StaticConstructor);

		new(GetClass(),TEXT("Product"               ),RF_Public)UStrProperty(CPP_PROPERTY(Product               ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("Version"               ),RF_Public)UStrProperty(CPP_PROPERTY(Version               ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("OldVersionInstallCheck"),RF_Public)UStrProperty(CPP_PROPERTY(OldVersionInstallCheck), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("OldVersionNumber"      ),RF_Public)UStrProperty(CPP_PROPERTY(OldVersionNumber      ), TEXT(""), CPF_Config   );
		new(GetClass(),TEXT("LocalProduct"          ),RF_Public)UStrProperty(CPP_PROPERTY(LocalProduct          ), TEXT(""), CPF_Localized);
		new(GetClass(),TEXT("Developer"             ),RF_Public)UStrProperty(CPP_PROPERTY(Developer             ), TEXT(""), CPF_Localized);
		new(GetClass(),TEXT("ProductURL"            ),RF_Public)UStrProperty(CPP_PROPERTY(ProductURL            ), TEXT(""), CPF_Localized);
		new(GetClass(),TEXT("VersionURL"            ),RF_Public)UStrProperty(CPP_PROPERTY(VersionURL            ), TEXT(""), CPF_Localized);
		new(GetClass(),TEXT("DeveloperURL"          ),RF_Public)UStrProperty(CPP_PROPERTY(DeveloperURL          ), TEXT(""), CPF_Localized);

		unguard;
	}
};

/*-----------------------------------------------------------------------------
	USetupGroup.
-----------------------------------------------------------------------------*/

class USetupGroup : public USetupObject
{
	DECLARE_CLASS(USetupGroup,USetupObject,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig)

	// Data variables.
	static USetupDefinition* Manager;
	FStringNoInit Caption, Description; // Localized.
	FStringNoInit MasterPath; // Per install.
	UBOOL Optional, Visible, Selected; // Per install.
	TArray<FString> File, Copy, Group, Folder, Backup, Delete, Ini, SaveIni, AddIni, Requires; // Per install.
	TArray<USetupGroup*> Subgroups; // In memory only.
   INT ExtraSpace;  // DEUS_EX STM

	// Internal variables.
	TMultiMap<FString,FString> UninstallLog;

	// Functions.
	void StaticConstructor()
	{
		guard(USetupProduct::StaticConstructor);
		UArrayProperty* P;

		  new(GetClass(),TEXT("Caption"        ),RF_Public)UStrProperty  (CPP_PROPERTY(Caption    ), TEXT(""), CPF_Config|CPF_Localized);
		  new(GetClass(),TEXT("Description"    ),RF_Public)UStrProperty  (CPP_PROPERTY(Description), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("MasterPath"     ),RF_Public)UStrProperty  (CPP_PROPERTY(MasterPath ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("Optional"       ),RF_Public)UBoolProperty (CPP_PROPERTY(Optional   ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("Visible"        ),RF_Public)UBoolProperty (CPP_PROPERTY(Visible    ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("Selected"       ),RF_Public)UBoolProperty (CPP_PROPERTY(Selected   ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("ExtraSpace"     ),RF_Public)UBoolProperty (CPP_PROPERTY(ExtraSpace ), TEXT(""), CPF_Config   );
		P=new(GetClass(),TEXT("File"           ),RF_Public)UArrayProperty(CPP_PROPERTY(File       ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Copy"           ),RF_Public)UArrayProperty(CPP_PROPERTY(Copy       ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Group"          ),RF_Public)UArrayProperty(CPP_PROPERTY(Group      ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Folder"         ),RF_Public)UArrayProperty(CPP_PROPERTY(Folder     ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Backup"         ),RF_Public)UArrayProperty(CPP_PROPERTY(Backup     ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Delete"         ),RF_Public)UArrayProperty(CPP_PROPERTY(Delete     ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Ini"            ),RF_Public)UArrayProperty(CPP_PROPERTY(Ini        ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("SaveIni"        ),RF_Public)UArrayProperty(CPP_PROPERTY(SaveIni    ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("AddIni"         ),RF_Public)UArrayProperty(CPP_PROPERTY(AddIni     ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;
		P=new(GetClass(),TEXT("Requires"       ),RF_Public)UArrayProperty(CPP_PROPERTY(Requires   ), TEXT(""), CPF_Config   );
        P->Inner = new(P,TEXT("StrProperty0"   ),RF_Public)UStrProperty;

		Optional = 0;
		Visible  = 0;
		Selected = 1;

		unguard;
	}
	USetupGroup();
	virtual SQWORD SpaceRequired();
};

/*-----------------------------------------------------------------------------
	USetupDefinition.
-----------------------------------------------------------------------------*/

struct FSavedIni
{
	FString File, Section, Key, SavedValue;
	FSavedIni( FString InFile, FString InSection, FString InKey, FString InSavedValue )
	: File(InFile), Section(InSection), Key(InKey), SavedValue(InSavedValue)
	{}
};

class USetupDefinition : public USetupProduct
{
	DECLARE_CLASS(USetupDefinition,USetupProduct,CLASS_Config|CLASS_Localized|CLASS_PerObjectConfig)

	// Per-product info.
	UBOOL					Patch;
	UBOOL					CdAutoPlay;
	UBOOL					IsMasterProduct;
	FStringNoInit			Language;
	FStringNoInit			Exe;
	FStringNoInit			PatchCdCheck;
	FStringNoInit			MasterProduct;
	TArray<FString>			Requires;
   TArray<FString>         PostExec;

	// Per-product, per-language info.
	FStringNoInit			DefaultFolder;
	FStringNoInit			License, ReadMe, Logo;
	FStringNoInit			SetupWindowTitle;
	FStringNoInit			AutoplayWindowTitle;

	// Internal variables.
	TArray<USetupGroup*>	UninstallComponents;
	USetupGroup*			RootGroup;
	FString					ConfigFile;
	FStringNoInit			DestPath;
	FStringNoInit			RefPath;
	FStringNoInit			SrcPath;
	FStringNoInit			CdOk;
	FStringNoInit			RegistryFolder;
	FStringNoInit			RegistryVersion;
	FStringNoInit			SetupIniFile;
	UBOOL					MustRestart;
	UBOOL					Exists;
	UBOOL					FolderExists;
	UBOOL					AnyRef;
	UBOOL					MustReboot;
	UBOOL					Uninstalling;
	UBOOL					NoRun;
	UBOOL					Manifest;
	INT						UninstallTotal;
	INT						UninstallCount;
	SQWORD					RequiredSpace;
	SQWORD					TotalBytes;
	SQWORD					RunningBytes;
	TArray<FSavedIni>		SavedIni;
	TMultiMap<FString,FString> RefCounts;

	// Interface.
	void StaticConstructor()
	{
		guard(USetupDefinition::StaticConstructor);
		UArrayProperty* P;

		  new(GetClass(),TEXT("Patch"              ),RF_Public)UBoolProperty (CPP_PROPERTY(Patch              ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("CdAutoPlay"         ),RF_Public)UBoolProperty (CPP_PROPERTY(CdAutoPlay         ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("IsMasterProduct"    ),RF_Public)UBoolProperty (CPP_PROPERTY(IsMasterProduct    ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("MasterProduct"      ),RF_Public)UStrProperty  (CPP_PROPERTY(MasterProduct      ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("Language"           ),RF_Public)UStrProperty  (CPP_PROPERTY(Language           ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("Exe"                ),RF_Public)UStrProperty  (CPP_PROPERTY(Exe                ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("PatchCdCheck"       ),RF_Public)UBoolProperty (CPP_PROPERTY(PatchCdCheck       ), TEXT(""), CPF_Config   );
		  new(GetClass(),TEXT("DefaultFolder"      ),RF_Public)UStrProperty  (CPP_PROPERTY(DefaultFolder      ), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("License"            ),RF_Public)UStrProperty  (CPP_PROPERTY(License            ), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("ReadMe"             ),RF_Public)UStrProperty  (CPP_PROPERTY(ReadMe             ), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("Logo"               ),RF_Public)UStrProperty  (CPP_PROPERTY(Logo               ), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("SetupWindowTitle"   ),RF_Public)UStrProperty  (CPP_PROPERTY(SetupWindowTitle   ), TEXT(""), CPF_Localized);
		  new(GetClass(),TEXT("AutoplayWindowTitle"),RF_Public)UStrProperty  (CPP_PROPERTY(AutoplayWindowTitle), TEXT(""), CPF_Localized);
		P=new(GetClass(),TEXT("Requires"           ),RF_Public)UArrayProperty(CPP_PROPERTY(Requires          ), TEXT(""), CPF_Config   );
		P->Inner = new(P,TEXT("StrProperty0"       ),RF_Public)UStrProperty;
		// DEUS_EX STM - added new property
		P=new(GetClass(),TEXT("PostExec"           ),RF_Public)UArrayProperty(CPP_PROPERTY(PostExec          ), TEXT(""), CPF_Config   );
		P->Inner = new(P,TEXT("StrProperty0"       ),RF_Public)UStrProperty;

		Language=TEXT("int");

		unguard;
	}
	USetupDefinition();
	virtual void Init();
 	virtual UBOOL LocateSourceFile( FString& Src );
	void Reformat( FString& Msg, TMultiMap<FString,FString>* Map );
	virtual FString Format( FString Msg, const TCHAR* Other=NULL );
	virtual INT UpdateRefCount( const TCHAR* Key, const TCHAR* Value, INT Inc );
	virtual void UninstallLogAdd( const TCHAR* Key, const TCHAR* Value, UBOOL Unique, UBOOL RefLog );
	virtual void LanguageChange();
	virtual FString GetFullRef( const FString RefFile );
	virtual void DidCancel();
	virtual void InstallTree( const TCHAR* Action, FInstallPoll* Poll, void (USetupDefinition::*Process)( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll ), USetupGroup* SetupGroup=NULL, UBOOL Selected=1 );
	virtual void UninstallTree( const TCHAR* Action, FInstallPoll* Poll, void (USetupDefinition::*Process)( FString Key, FString Value, FInstallPoll* Poll ) );
	virtual void SetupFormatStrings();
	virtual void BeginSteps();
	virtual void EndSteps();
	virtual void DoInstallSteps( FInstallPoll* Poll );
	virtual void DoUninstallSteps( FInstallPoll* Poll );
	virtual void PreExit();
	virtual UBOOL GetRegisteredProductFolder( FString Product, FString& Folder );
	virtual void PerformUninstallCopy();
	virtual void CreateRootGroup();
	virtual UBOOL CheckRequirement( FString Folder, USetupProduct* RequiredProduct, FString& FailMessage );
	virtual UBOOL CheckAllRequirements( FString Folder, USetupProduct*& FailedRequiredProduct, FString& FailMessage );

	// Installation steps.
	virtual void ProcessCheckRef( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	virtual void ProcessVerifyCd( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	virtual void ProcessPreCopy( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	virtual void ProcessCopy( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	virtual void ProcessExtra( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );
	virtual void ProcessPostCopy( FString Key, FString Value, UBOOL Selected, FInstallPoll* Poll );

	// Uninstall steps.
	virtual void ProcessUninstallCountTotal( FString Key, FString Value, FInstallPoll* Poll );
	virtual void ProcessUninstallRemove( FString Key, FString Value, FInstallPoll* Poll );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
