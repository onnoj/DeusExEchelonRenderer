/*=============================================================================
	UnLinker.h: Unreal object linker.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Hash function.
-----------------------------------------------------------------------------*/

inline INT HashNames( FName A, FName B, FName C )
{
	if( C==NAME_UnrealShare )//oldver
		C=NAME_UnrealI;
	return A.GetIndex() + 7 * B.GetIndex() + 31*C.GetIndex();
}

/*-----------------------------------------------------------------------------
	FObjectExport.
-----------------------------------------------------------------------------*/

//
// Information about an exported object.
//
struct CORE_API FObjectExport
{
	// Variables.
	INT         ClassIndex;		// Persistent.
	INT         SuperIndex;		// Persistent (for UStruct-derived objects only).
	INT			PackageIndex;	// Persistent.
	FName		ObjectName;		// Persistent.
	DWORD		ObjectFlags;	// Persistent.
	INT         SerialSize;		// Persistent.
	INT         SerialOffset;	// Persistent (for checking only).
	UObject*	_Object;		// Internal.
	INT			_iHashNext;		// Internal.

	// Functions.
	FObjectExport()
	:	_Object			( NULL														)
	,	_iHashNext		( INDEX_NONE												)
	{}
	FObjectExport( UObject* InObject )
	:	ClassIndex		( 0															)
	,	SuperIndex		( 0															)
	,	PackageIndex	( 0															)
	,	ObjectName		( InObject ? (InObject->GetFName()			) : NAME_None	)
	,	ObjectFlags		( InObject ? (InObject->GetFlags() & RF_Load) : 0			)
	,	SerialSize		( 0															)
	,	SerialOffset	( 0															)
	,	_Object			( InObject													)
	{}
	friend FArchive& operator<<( FArchive& Ar, FObjectExport& E )
	{
		guard(FObjectExport<<);

		Ar << AR_INDEX(E.ClassIndex);
		Ar << AR_INDEX(E.SuperIndex);
		Ar << E.PackageIndex;
		Ar << E.ObjectName;
		Ar << E.ObjectFlags;
		Ar << AR_INDEX(E.SerialSize);
		if( E.SerialSize )
			Ar << AR_INDEX(E.SerialOffset);

		return Ar;
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FObjectImport.
-----------------------------------------------------------------------------*/

//
// Information about an imported object.
//
struct CORE_API FObjectImport
{
	// Variables.
	FName			ClassPackage;	// Persistent.
	FName			ClassName;		// Persistent.
	INT				PackageIndex;	// Persistent.
	FName			ObjectName;		// Persistent.
	UObject*		XObject;		// Internal (only really needed for saving, can easily be gotten rid of for loading).
	ULinkerLoad*	SourceLinker;	// Internal.
	INT             SourceIndex;	// Internal.

	// Functions.
	FObjectImport()
	{}
	FObjectImport( UObject* InObject )
	:	ClassPackage	( InObject->GetClass()->GetOuter()->GetFName())
	,	ClassName		( InObject->GetClass()->GetFName()		 )
	,	PackageIndex	( 0                                      )
	,	ObjectName		( InObject->GetFName()					 )
	,	XObject			( InObject								 )
	,	SourceLinker	( NULL									 )
	,	SourceIndex		( INDEX_NONE							 )
	{
			if( XObject )
				UObject::GImportCount++;
	}
	friend FArchive& operator<<( FArchive& Ar, FObjectImport& I )
	{
		guard(FObjectImport<<);

		Ar << I.ClassPackage << I.ClassName;
		Ar << I.PackageIndex;
		Ar << I.ObjectName;
		if( Ar.IsLoading() )
		{
			I.SourceIndex = INDEX_NONE;
			I.XObject     = NULL;
		}
		return Ar;

		unguard;
	}
};

/*----------------------------------------------------------------------------
	Items stored in Unrealfiles.
----------------------------------------------------------------------------*/

//
// Unrealfile summary, stored at top of file.
//
struct FGenerationInfo
{
	INT ExportCount, NameCount;
	FGenerationInfo( INT InExportCount, INT InNameCount )
	: ExportCount(InExportCount), NameCount(InNameCount)
	{}
	friend FArchive& operator<<( FArchive& Ar, FGenerationInfo& Info )
	{
		guard(FGenerationInfo<<);
		return Ar << Info.ExportCount << Info.NameCount;
		unguard;
	}
};
struct FPackageFileSummary
{
	// Variables.
	INT		Tag;
	INT		FileVersion;
	DWORD	PackageFlags;
	INT		NameCount,		NameOffset;
	INT		ExportCount,	ExportOffset;
	INT     ImportCount,	ImportOffset;
	FGuid	Guid;
	TArray<FGenerationInfo> Generations;

	// Constructor.
	FPackageFileSummary()
	{
		appMemzero( this, sizeof(*this) );
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FPackageFileSummary& Sum )
	{
		guard(FUnrealfileSummary<<);

		Ar << Sum.Tag;
		Ar << Sum.FileVersion;
		Ar << Sum.PackageFlags;
		Ar << Sum.NameCount     << Sum.NameOffset;
		Ar << Sum.ExportCount   << Sum.ExportOffset;
		Ar << Sum.ImportCount   << Sum.ImportOffset;
		if( Sum.FileVersion>=68 )
		{
			INT GenerationCount = Sum.Generations.Num();
			Ar << Sum.Guid << GenerationCount;
			//!!67 had: return
			if( Ar.IsLoading() )
				Sum.Generations = TArray<FGenerationInfo>( GenerationCount );
			for( INT i=0; i<GenerationCount; i++ )
				Ar << Sum.Generations(i);
		}
		else //oldver
		{
			INT HeritageCount, HeritageOffset;
			Ar << HeritageCount << HeritageOffset;
			INT Saved = Ar.Tell();
			if( HeritageCount )
			{
				Ar.Seek( HeritageOffset );
				for( INT i=0; i<HeritageCount; i++ )
					Ar << Sum.Guid;
			}
			Ar.Seek( Saved );
			if( Ar.IsLoading() )
			{
				Sum.Generations.Empty( 1 );
				new(Sum.Generations)FGenerationInfo(Sum.ExportCount,Sum.NameCount);
			}
		}

		return Ar;
		unguard;
	}
};

/*----------------------------------------------------------------------------
	ULinker.
----------------------------------------------------------------------------*/

//
// A file linker.
//
class CORE_API ULinker : public UObject
{
	DECLARE_CLASS(ULinker,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(ULinker)

	// Variables.
	UObject*				LinkerRoot;			// The linker's root object.
	FPackageFileSummary		Summary;			// File summary.
	TArray<FName>			NameMap;			// Maps file name indices to name table indices.
	TArray<FObjectImport>	ImportMap;			// Maps file object indices >=0 to external object names.
	TArray<FObjectExport>	ExportMap;			// Maps file object indices >=0 to external object names.
	INT						Success;			// Whether the object was constructed successfully.
	FString					Filename;			// Filename.
	DWORD					_ContextFlags;		// Load flag mask.

	// Constructors.
	ULinker( UObject* InRoot, const TCHAR* InFilename )
	:	LinkerRoot( InRoot )
	,	Summary()
	,	Success( 123456 )
	,	Filename( InFilename )
	,	_ContextFlags( 0 )
	{
		check(LinkerRoot);
		check(InFilename);

		// Set context flags.
		if( GIsEditor ) _ContextFlags |= RF_LoadForEdit;
		if( GIsClient ) _ContextFlags |= RF_LoadForClient;
		if( GIsServer ) _ContextFlags |= RF_LoadForServer;
	}

	// UObject interface.
	void Serialize( FArchive& Ar )
	{
		guard(ULinker::Serialize);
		Super::Serialize( Ar );

		// Sizes.
		ImportMap	.CountBytes( Ar );
		ExportMap	.CountBytes( Ar );

		// Prevent garbage collecting of linker's names and package.
		Ar << NameMap << LinkerRoot;
		{for( INT i=0; i<ExportMap.Num(); i++ )
		{
			FObjectExport& E = ExportMap(i);
			Ar << E.ObjectName;
		}}
		{for( INT i=0; i<ImportMap.Num(); i++ )
		{
			FObjectImport& I = ImportMap(i);
			Ar << *(UObject**)&I.SourceLinker;
			Ar << I.ClassPackage << I.ClassName;
		}}
		unguard;
	}

	// ULinker interface.
	FString GetImportFullName( INT i )
	{
		guard(ULinkerLoad::GetImportFullName);
		FString S;
		for( INT j=-i-1; j!=0; j=ImportMap(-j-1).PackageIndex )
		{
			if( j != -i-1 )
				S = US + TEXT(".") + S;
			S = FString(*ImportMap(-j-1).ObjectName) + S;
		}
		return FString(*ImportMap(i).ClassName) + TEXT(" ") + S ;
		unguard;
	}
	FString GetExportFullName( INT i, const TCHAR* FakeRoot=NULL )
	{
		guard(ULinkerLoad::GetExportFullName);
		FString S;
		for( INT j=i+1; j!=0; j=ExportMap(j-1).PackageIndex )
		{
			if( j != i+1 )
				S = US + TEXT(".") + S;
			S = FString(*ExportMap(j-1).ObjectName) + S;
		}
		INT ClassIndex = ExportMap(i).ClassIndex;
		FName ClassName = ClassIndex>0 ? ExportMap(ClassIndex-1).ObjectName : ClassIndex<0 ? ImportMap(-ClassIndex-1).ObjectName : NAME_Class;
		return FString(*ClassName) + TEXT(" ") + (FakeRoot ? FakeRoot : LinkerRoot->GetPathName()) + TEXT(".") + S;
		unguard;
	}
};

/*----------------------------------------------------------------------------
	ULinkerLoad.
----------------------------------------------------------------------------*/

//
// A file loader.
//
class ULinkerLoad : public ULinker, public FArchive
{
	DECLARE_CLASS(ULinkerLoad,ULinker,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(ULinkerLoad)

	// Friends.
	friend class UObject;
	friend class UPackageMap;

	// Variables.
	DWORD					LoadFlags;
	UBOOL					Verified;
	INT						ExportHash[256];
	TArray<FLazyLoader*>	LazyLoaders;
	FArchive*				Loader;

	// Constructor; all errors here throw exceptions which are fully recoverable.
	ULinkerLoad( UObject* InParent, const TCHAR* InFilename, DWORD InLoadFlags )
	:	ULinker( InParent, InFilename )
	,	LoadFlags( InLoadFlags )
	{
		guard(ULinkerLoad::ULinkerLoad);

		if(!(LoadFlags & LOAD_Quiet)) 
			debugf( TEXT("Loading: %s"), InParent->GetFullName() );

		Loader = GFileManager->CreateFileReader( InFilename, 0, GError );
		if( !Loader )
			appThrowf( LocalizeError("OpenFailed") );

		// Error if linker already loaded.
		{for( INT i=0; i<GObjLoaders.Num(); i++ )
			if( GetLoader(i)->LinkerRoot == LinkerRoot )
				appThrowf( LocalizeError("LinkerExists"), LinkerRoot->GetName() );}

		// Begin.
		GWarn->StatusUpdatef( 0, 0, LocalizeProgress("Loading"), *Filename );

		// Set status info.
		guard(InitAr);
		ArVer       = PACKAGE_FILE_VERSION;
		ArIsLoading = ArIsPersistent = 1;
		ArForEdit   = GIsEditor;
		ArForClient = 1;
		ArForServer = 1;
		unguard;

		// Read summary from file.
		guard(LoadSummary);
		*this << Summary;
		ArVer = Summary.FileVersion;
		if( Cast<UPackage>(LinkerRoot) )
			Cast<UPackage>(LinkerRoot)->PackageFlags = Summary.PackageFlags;
		unguard;

		// Check tag.
		guard(CheckTag);
		if( Summary.Tag != PACKAGE_FILE_TAG )
		{
			GWarn->Logf( LocalizeError("BinaryFormat"), *Filename );
			throw( LocalizeError("Aborted") );
		}
		unguard;

		// Validate the summary.
		guard(CheckVersion);
		if( Summary.FileVersion < PACKAGE_MIN_VERSION )
			if( !GWarn->YesNof( LocalizeQuery("OldVersion"), *Filename ) )
				throw( LocalizeError("Aborted") );
		unguard;

		// Slack everything according to summary.
		ImportMap   .Empty( Summary.ImportCount   );
		ExportMap   .Empty( Summary.ExportCount   );
		NameMap		.Empty( Summary.NameCount     );

		// Load and map names.
		guard(LoadNames);
		if( Summary.NameCount > 0 )
		{
			Seek( Summary.NameOffset );
			for( INT i=0; i<Summary.NameCount; i++ )
			{
				// Read the name entry from the file.
				FNameEntry NameEntry;
				*this << NameEntry;

				// Add it to the name table if it's needed in this context.				
				NameMap.AddItem( (NameEntry.Flags & _ContextFlags) ? FName( NameEntry.Name, FNAME_Add ) : NAME_None );
			}
		}
		unguard;

		// Load import map.
		guard(LoadImportMap);
		if( Summary.ImportCount > 0 )
		{
			Seek( Summary.ImportOffset );
			for( INT i=0; i<Summary.ImportCount; i++ )
				*this << *new(ImportMap)FObjectImport;
		}
		unguard;

		// Load export map.
		guard(LoadExportMap);
		if( Summary.ExportCount > 0 )
		{
			Seek( Summary.ExportOffset );
			for( INT i=0; i<Summary.ExportCount; i++ )
				*this << *new(ExportMap)FObjectExport;
		}
		unguard;

		// Create export hash.
		//warning: Relies on import & export tables, so must be done here.
		{for( INT i=0; i<ARRAY_COUNT(ExportHash); i++ )
		{
			ExportHash[i] = INDEX_NONE;
		}}
		{for( INT i=0; i<ExportMap.Num(); i++ )
		{
			INT iHash = HashNames( ExportMap(i).ObjectName, GetExportClassName(i), GetExportClassPackage(i) ) & (ARRAY_COUNT(ExportHash)-1);
			ExportMap(i)._iHashNext = ExportHash[iHash];
			ExportHash[iHash] = i;
		}}

		// Add this linker to the object manager's linker array.
		GObjLoaders.AddItem( this );
		if( !(LoadFlags & LOAD_NoVerify) )
			Verify();

		// Success.
		Success = 1;

		unguard;
	}
	void Verify()
	{
		guard(ULinkerLoad::Verify);
		if( !Verified )
		{
			if( Cast<UPackage>(LinkerRoot) )
				Cast<UPackage>(LinkerRoot)->PackageFlags &= ~PKG_BrokenLinks;
			try
			{
				// Validate all imports and map them to their remote linkers.
				guard(ValidateImports);
				for( INT i=0; i<Summary.ImportCount; i++ )
					VerifyImport( i );
				unguard;
			}
			catch( TCHAR* Error )
			{
				GObjLoaders.RemoveItem( this );
				throw( Error );
			}
		}
		Verified=1;
		unguard;
	}
	FName GetExportClassPackage( INT i )
	{
		guardSlow(ULinkerLoad::GetExportClassPackage);
		FObjectExport& Export = ExportMap( i );
		if( Export.ClassIndex < 0 )
		{
			FObjectImport& Import = ImportMap( -Export.ClassIndex-1 );
			checkSlow(Import.PackageIndex<0);
			return ImportMap( -Import.PackageIndex-1 ).ObjectName;
		}
		else if( Export.ClassIndex > 0 )
		{
			return LinkerRoot->GetFName();
		}
		else
		{
			return NAME_Core;
		}
		unguardSlow;
	}
	FName GetExportClassName( INT i )
	{
		guardSlow(GetExportClassName);
		FObjectExport& Export = ExportMap(i);
		if( Export.ClassIndex < 0 )
		{
			return ImportMap( -Export.ClassIndex-1 ).ObjectName;
		}
		else if( Export.ClassIndex > 0 )
		{
			return ExportMap( Export.ClassIndex-1 ).ObjectName;
		}
		else
		{
			return NAME_Class;
		}
		unguardSlow;
	}

	// Safely verify an import.
	void VerifyImport( INT i )
	{
		guard(ULinkerLoad::VerifyImport);
		SharewareHack://oldver
		FObjectImport& Import = ImportMap(i);
		if
		(	Import.SourceIndex	!= INDEX_NONE
		||	Import.ClassPackage	== NAME_None
		||	Import.ClassName	== NAME_None
		||	Import.ObjectName	== NAME_None )
		{
			// Already verified, or not relevent in this context.
			return;
		}

		// Find or load this import's linker.
		INT Depth=0;
		UObject* Pkg=NULL;
		if( Import.PackageIndex == 0 )
		{
			check(Import.ClassName==NAME_Package);
			check(Import.ClassPackage==NAME_Core);
			UPackage* TmpPkg = CreatePackage( NULL, *Import.ObjectName );
			SharewareKludge://oldver
			try
			{
				Import.SourceLinker = GetPackageLinker( TmpPkg, NULL, LOAD_Throw | (LoadFlags & LOAD_Propagate), NULL, NULL );
			}
			catch( const TCHAR* Error )//oldver
			{
				if( TmpPkg->GetFName()==NAME_UnrealI )
				{
					TmpPkg = CreatePackage( NULL, TEXT("UnrealShare") );
					goto SharewareKludge;
				}
				appThrowf( Error );
			}
		}
		else
		{
			check(Import.PackageIndex<0);
			VerifyImport( -Import.PackageIndex-1 );
			Import.SourceLinker = ImportMap(-Import.PackageIndex-1).SourceLinker;
			check(Import.SourceLinker);
			FObjectImport* Top;
			for
			(	Top = &Import
			;	Top->PackageIndex<0
			;	Top = &ImportMap(-Top->PackageIndex-1),Depth++ );
			Pkg = CreatePackage( NULL, *Top->ObjectName );
		}

		// Find this import within its existing linker.
		UBOOL SafeReplace = 0;
	Rehack://oldver
		//new:
		INT iHash = HashNames( Import.ObjectName, Import.ClassName, Import.ClassPackage) & (ARRAY_COUNT(ExportHash)-1);
		for( INT j=Import.SourceLinker->ExportHash[iHash]; j!=INDEX_NONE; j=Import.SourceLinker->ExportMap(j)._iHashNext )
		//old:
		//for( INT j=0; j<Import.SourceLinker->ExportMap.Num(); j++ )
		{
			FObjectExport& Source = Import.SourceLinker->ExportMap( j );
			UBOOL ClassHack = Import.ClassPackage==NAME_UnrealI && Import.SourceLinker->GetExportClassPackage(j)==NAME_UnrealShare;//oldver
			if
			(	(Source.ObjectName	                          ==Import.ObjectName               )
			&&	(Import.SourceLinker->GetExportClassName   (j)==Import.ClassName                )
			&&  (Import.SourceLinker->GetExportClassPackage(j)==Import.ClassPackage || ClassHack) )
			{
				if( Import.PackageIndex<0 )
				{
					FObjectImport& ParentImport = ImportMap(-Import.PackageIndex-1);
					if( ParentImport.SourceLinker )
					{
						if( ParentImport.SourceIndex==INDEX_NONE )
						{
							if( Source.PackageIndex!=0 )
							{
								continue;
							}
						}
						else if( ParentImport.SourceIndex+1 != Source.PackageIndex )
						{
							if( Source.PackageIndex!=0 )
							{
								continue;
							}
						}
					}
				}
				if( !(Source.ObjectFlags & RF_Public) )
				{
					if( LoadFlags & LOAD_Forgiving )
					{
						if( Cast<UPackage>(LinkerRoot) )
							Cast<UPackage>(LinkerRoot)->PackageFlags |= PKG_BrokenLinks;
						debugf( TEXT("Broken import: %s %s (file %s)"), *Import.ClassName, *GetImportFullName(i), *Import.SourceLinker->Filename );
						return;
					}
					appThrowf( LocalizeError("FailedImportPrivate"), *Import.ClassName, *GetImportFullName(i) );
				}
				Import.SourceIndex = j;
				break;
			}
		}
		if( appStricmp(*Import.ClassName,TEXT("Mesh"))==0 )//oldver
		{
			Import.ClassName=FName(TEXT("LodMesh"));
			goto Rehack;
		}

		// If not found in file, see if it's a public native transient class.
		if( Import.SourceIndex==INDEX_NONE && Pkg!=NULL )
		{
			UObject* ClassPackage = FindObject<UPackage>( NULL, *Import.ClassPackage );
			if( ClassPackage )
			{
				UClass* FindClass = FindObject<UClass>( ClassPackage, *Import.ClassName );
				if( FindClass )
				{
					UObject* FindObject = StaticFindObject( FindClass, Pkg, *Import.ObjectName );
					if
					(	(FindObject)
					&&	(FindObject->GetFlags() & RF_Public)
					&&	(FindObject->GetFlags() & RF_Native)
					&&	(FindObject->GetFlags() & RF_Transient) )
					{
						Import.XObject = FindObject;
						GImportCount++;
					}
					else if( FindClass->ClassFlags & CLASS_SafeReplace )
					{
						if( GCheckConflicts )
							debugf( TEXT("Missing %s %s"), FindClass->GetName(), *GetImportFullName(i) );
						SafeReplace = 1;
					}
				}
			}
			if( !Import.XObject && Pkg!=NULL && Pkg->GetFName()==NAME_UnrealI && Depth==1 )//oldver
			{
				Import.PackageIndex = -ImportMap.Num()-1;
				FObjectImport& New  = *new(ImportMap)FObjectImport;
				New.ClassPackage	= NAME_Core;
				New.ClassName		= NAME_Package;
				New.PackageIndex	= 0;
				New.ObjectName		= NAME_UnrealShare;
				New.XObject			= NULL;
				New.SourceLinker	= NULL;
				New.SourceIndex		= INDEX_NONE;
				VerifyImport(ImportMap.Num()-1);
				goto SharewareHack;
			}
			if( !Import.XObject && !SafeReplace )
			{
				if( LoadFlags & LOAD_Forgiving )
				{
					if( Cast<UPackage>(LinkerRoot) )
						Cast<UPackage>(LinkerRoot)->PackageFlags |= PKG_BrokenLinks;
					debugf( TEXT("Broken import: %s %s (file %s)"), *Import.ClassName, *GetImportFullName(i), *Import.SourceLinker->Filename );
					return;
				}
				appThrowf( LocalizeError("FailedImport"), *Import.ClassName, *GetImportFullName(i) );
			}
		}
		unguard;
	}

	// Load all objects; all errors here are fatal.
	void LoadAllObjects()
	{
		guard(ULinkerLoad::LoadAllObjects);
		for( INT i=0; i<Summary.ExportCount; i++ )
			CreateExport( i );
		unguardobj;
	}

	// Find the index of a specified object.
	//!!without regard to specific package
	INT FindExportIndex( FName ClassName, FName ClassPackage, FName ObjectName, INT PackageIndex )
	{
		guard(ULinkerLoad::FindExportIndex);
	Rehack://oldver
		INT iHash = HashNames( ObjectName, ClassName, ClassPackage ) & (ARRAY_COUNT(ExportHash)-1);
		for( INT i=ExportHash[iHash]; i!=INDEX_NONE; i=ExportMap(i)._iHashNext )
		{
			if
			(  (ExportMap(i).ObjectName  ==ObjectName                              )
			&& (ExportMap(i).PackageIndex==PackageIndex || PackageIndex==INDEX_NONE)
			&& (GetExportClassPackage(i) ==ClassPackage                            )
			&& (GetExportClassName   (i) ==ClassName                               ) )
			{
				return i;
			}
		}
		if( appStricmp(*ClassName,TEXT("Mesh"))==0 )//oldver.
		{
			ClassName = FName(TEXT("LodMesh"));
			goto Rehack;
		}
		return INDEX_NONE;
		unguard;
	}

	// Create a single object.
	UObject* Create( UClass* ObjectClass, FName ObjectName, DWORD LoadFlags, UBOOL Checked )
	{
		guard(ULinkerLoad::Create);
		//old:
		//for( INT i=0; i<ExportMap.Num(); i++ )
		//new:
		INT Index = FindExportIndex( ObjectClass->GetFName(), ObjectClass->GetOuter()->GetFName(), ObjectName, INDEX_NONE );
		if( Index!=INDEX_NONE )
			return (LoadFlags & LOAD_Verify) ? (UObject*)-1 : CreateExport(Index);
		if( Checked )
			appThrowf( LocalizeError("FailedCreate"), ObjectClass->GetName(), *ObjectName );
		return NULL;
		unguard;
	}
	void Preload( UObject* Object )
	{
		guard(ULinkerLoad::Preload);
		check(IsValid());
		check(Object);
		if( Object->GetFlags() & RF_Preloading )
		{
			// Warning for internal development.
			//debugf( "Object preload reentrancy: %s", Object->GetFullName() );
		}
		if( Object->GetLinker()==this )
		{
			// Preload the object if necessary.
			if( Object->GetFlags() & RF_NeedLoad )
			{
				// If this is a struct, preload its super.
				if(	Object->IsA(UStruct::StaticClass()) )
					if( ((UStruct*)Object)->SuperField )
						Preload( ((UStruct*)Object)->SuperField );

				// Load the local object now.
				guard(LoadObject);
				FObjectExport& Export = ExportMap( Object->_LinkerIndex );
				check(Export._Object==Object);
				INT SavedPos = Loader->Tell();
				Loader->Seek( Export.SerialOffset );
				Loader->Precache( Export.SerialSize );

				// Load the object.
				Object->ClearFlags ( RF_NeedLoad );
				Object->SetFlags   ( RF_Preloading );
				Object->Serialize  ( *this );
				Object->ClearFlags ( RF_Preloading );
				//debugf(NAME_Log,"    %s: %i", Object->GetFullName(), Export.SerialSize );

				// Make sure we serialized the right amount of stuff.
				if( Tell()-Export.SerialOffset != Export.SerialSize )
					appErrorf( LocalizeError("SerialSize"), Object->GetFullName(), Tell()-Export.SerialOffset, Export.SerialSize );
				Loader->Seek( SavedPos );
				unguardf(( TEXT("(%s %i==%i/%i %i %i)"), Object->GetFullName(), Loader->Tell(), Loader->Tell(), Loader->TotalSize(), ExportMap( Object->_LinkerIndex ).SerialOffset, ExportMap( Object->_LinkerIndex ).SerialSize ));
			}
		}
		else if( Object->GetLinker() )
		{
			// Send to the object's linker.
			Object->GetLinker()->Preload( Object );
		}
		unguard;
	}

private:
	// Return the loaded object corresponding to an export index; any errors are fatal.
	UObject* CreateExport( INT Index )
	{
		guard(ULinkerLoad::CreateExport);

		// Map the object into our table.
		FObjectExport& Export = ExportMap( Index );
		if( !Export._Object && (Export.ObjectFlags & _ContextFlags) )
		{
			check(Export.ObjectName!=NAME_None || !(Export.ObjectFlags&RF_Public));

			// Get the object's class.
			UClass* LoadClass = (UClass*)IndexToObject( Export.ClassIndex );
			if( !LoadClass )
				LoadClass = UClass::StaticClass();
			check(LoadClass);
			check(LoadClass->GetClass()==UClass::StaticClass());
			if( LoadClass->GetFName()==NAME_Camera )//oldver
				return NULL;
			Preload( LoadClass );

			// Get the outer object. If that caused the object to load, return it.
			UObject* ThisParent = Export.PackageIndex ? IndexToObject(Export.PackageIndex) : LinkerRoot;
			if( Export._Object )
				return Export._Object;

			//oldver: Move actors from root to level.
			/*if( Ver() <= 61 )
			{
				static UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE,"Actor");
				static UClass* LevelClass = FindObject<UClass>(ANY_PACKAGE,"Level");
				if( ActorClass && LoadClass->IsChildOf(ActorClass) && ThisParent==LinkerRoot )
					ThisParent = StaticFindObjectChecked( LevelClass, LinkerRoot, "MyLevel" );
			}*/

			// Create the export object.
			Export._Object = StaticConstructObject
			(
				LoadClass,
				ThisParent,
				Export.ObjectName,
				(Export.ObjectFlags & RF_Load) | RF_NeedLoad | RF_NeedPostLoad
			);
			Export._Object->SetLinker( this, Index );
			GObjLoaded.AddItem( Export._Object );
			debugfSlow( NAME_DevLoad, TEXT("Created %s"), Export._Object->GetFullName() );

			// If it's a struct or class, set its parent.
			if( Export._Object->IsA(UStruct::StaticClass()) && Export.SuperIndex!=0 )
				((UStruct*)Export._Object)->SuperField = (UStruct*)IndexToObject( Export.SuperIndex );

			// If it's a class, bind it to C++.
			if( Export._Object->IsA( UClass::StaticClass() ) )
				((UClass*)Export._Object)->Bind();
		}
		return Export._Object;
		unguardf(( TEXT("(%s %i)"), *ExportMap(Index).ObjectName, Tell() ));
	}

	// Return the loaded object corresponding to an import index; any errors are fatal.
	UObject* CreateImport( INT Index )
	{
		guard(ULinkerLoad::CreateImport);
		FObjectImport& Import = ImportMap( Index );
		if( !Import.XObject && Import.SourceIndex>=0 )
		{
			//debugf( "Imported new %s %s.%s", *Import.ClassName, *Import.ObjectPackage, *Import.ObjectName );
			check(Import.SourceLinker);
			Import.XObject = Import.SourceLinker->CreateExport( Import.SourceIndex );
			GImportCount++;
		}
		return Import.XObject;
		unguard;
	}

	// Map an import/export index to an object; all errors here are fatal.
	UObject* IndexToObject( INT Index )
	{
		guard(IndexToObject);
		if( Index > 0 )
		{
			if( !ExportMap.IsValidIndex( Index-1 ) )
				appErrorf( LocalizeError("ExportIndex"), Index-1, ExportMap.Num() );			
			return CreateExport( Index-1 );
		}
		else if( Index < 0 )
		{
			if( !ImportMap.IsValidIndex( -Index-1 ) )
				appErrorf( LocalizeError("ImportIndex"), -Index-1, ImportMap.Num() );
			return CreateImport( -Index-1 );
		}
		else return NULL;
		unguard;
	}

	// Detach an export from this linker.
	void DetachExport( INT i )
	{
		guard(ULinkerLoad::DetachExport);
		FObjectExport& E = ExportMap( i );
		check(E._Object);
		if( !E._Object->IsValid() )
			appErrorf( TEXT("Linker object %s %s.%s is invalid"), *GetExportClassName(i), LinkerRoot->GetName(), *E.ObjectName );
		if( E._Object->GetLinker()!=this )
			appErrorf( TEXT("Linker object %s %s.%s mislinked"), *GetExportClassName(i), LinkerRoot->GetName(), *E.ObjectName );
		if( E._Object->_LinkerIndex!=i )
			appErrorf( TEXT("Linker object %s %s.%s misindexed"), *GetExportClassName(i), LinkerRoot->GetName(), *E.ObjectName );
		ExportMap(i)._Object->SetLinker( NULL, INDEX_NONE );
		unguard;
	}

	// UObject interface.
	void Serialize( FArchive& Ar )
	{
		guard(ULinkerLoad::Serialize);
		Super::Serialize( Ar );
		LazyLoaders.CountBytes( Ar );
		unguard;
	}
	void Destroy()
	{
		guard(ULinkerLoad::Destroy);

		if(!(LoadFlags & LOAD_Quiet)) 
			debugf( TEXT("Unloading: %s"), LinkerRoot->GetFullName() );

		// Detach all lazy loaders.
		DetachAllLazyLoaders( 0 );

		// Detach all objects linked with this linker.
		for( INT i=0; i<ExportMap.Num(); i++ )
			if( ExportMap(i)._Object )
				DetachExport( i );

		// Remove from object manager, if it has been added.
		GObjLoaders.RemoveItem( this );
		if( Loader )
			delete Loader;
		Loader = NULL;

		Super::Destroy();
		unguardobj;
	}

	// FArchive interface.
	void AttachLazyLoader( FLazyLoader* LazyLoader )
	{
		guard(ULinkerLoad::AttachLazyLoader);
		checkSlow(LazyLoader->SavedAr==NULL);
		checkSlow(LazyLoaders.FindItemIndex(LazyLoader)==INDEX_NONE);

		LazyLoaders.AddItem( LazyLoader );
		LazyLoader->SavedAr  = this;
		LazyLoader->SavedPos = Tell();

		unguard;
	}
	void DetachLazyLoader( FLazyLoader* LazyLoader )
	{
		guard(ULinkerLoad::DetachLazyLoader);
		checkSlow(LazyLoader->SavedAr==this);

		INT RemovedCount = LazyLoaders.RemoveItem(LazyLoader);
		if( RemovedCount!=1 )
			appErrorf( TEXT("Detachment inconsistency: %i (%s)"), RemovedCount, *Filename );
		LazyLoader->SavedAr = NULL;
		LazyLoader->SavedPos = 0;

		unguard;
	}
	void DetachAllLazyLoaders( UBOOL Load )
	{
		guard(ULinkerLoad::DetachAllLazyLoaders);
		for( INT i=0; i<LazyLoaders.Num(); i++ )
		{
			FLazyLoader* LazyLoader = LazyLoaders( i );
			if( Load )
				LazyLoader->Load();
			LazyLoader->SavedAr  = NULL;
			LazyLoader->SavedPos = 0;
		}
		LazyLoaders.Empty();
		unguard;
	}

	// FArchive interface.
	void Seek( INT InPos )
	{
		guard(ULinkerLoad::Seek);
		Loader->Seek( InPos );
		unguard;
	}
	INT Tell()
	{
		guard(ULinkerLoad::Tell);
		return Loader->Tell();
		unguard;
	}
	INT TotalSize()
	{
		guard(ULinkerLoad::TotalSize);
		return Loader->TotalSize();
		unguard;
	}
	void Serialize( void* V, INT Length )
	{
		guard(ULinkerLoad::Serialize);
		Loader->Serialize( V, Length );
		unguard;
	}
	FArchive& operator<<( UObject*& Object )
	{
		guard(ULinkerLoad<<UObject);

		INT Index;
		*Loader << AR_INDEX(Index);
		Object = IndexToObject( Index );

		return *this;
		unguardf(( TEXT("(%s %i))"), GetFullName(), Tell() ));
	}
	FArchive& operator<<( FName& Name )
	{
		guard(ULinkerLoad<<FName);

		NAME_INDEX NameIndex;
		*Loader << AR_INDEX(NameIndex);

		if( !NameMap.IsValidIndex(NameIndex) )
			appErrorf( TEXT("Bad name index %i/%i"), NameIndex, NameMap.Num() );	
		Name = NameMap( NameIndex );

		return *this;
		unguardf(( TEXT("(%s %i))"), GetFullName(), Tell() ));
	}
};

/*----------------------------------------------------------------------------
	ULinkerSave.
----------------------------------------------------------------------------*/

//
// A file saver.
//
class ULinkerSave : public ULinker, public FArchive
{
	DECLARE_CLASS(ULinkerSave,ULinker,CLASS_Transient);
	NO_DEFAULT_CONSTRUCTOR(ULinkerSave);

	// Variables.
	FArchive* Saver;
	TArray<INT> ObjectIndices;
	TArray<INT> NameIndices;

	// Constructor.
	ULinkerSave( UObject* InParent, const TCHAR* InFilename )
	:	ULinker( InParent, InFilename )
	,	Saver( NULL )
	{
		// Create file saver.
		Saver = GFileManager->CreateFileWriter( InFilename, 0, GThrow );
		if( !Saver )
			appThrowf( LocalizeError("OpenFailed") );

		// Set main summary info.
		Summary.Tag           = PACKAGE_FILE_TAG;
		Summary.FileVersion	  = PACKAGE_FILE_VERSION;
		Summary.PackageFlags  = Cast<UPackage>(LinkerRoot) ? Cast<UPackage>(LinkerRoot)->PackageFlags : 0;

		// Set status info.
		ArIsSaving     = 1;
		ArIsPersistent = 1;
		ArForEdit      = GIsEditor;
		ArForClient    = 1;
		ArForServer    = 1;

		// Allocate indices.
		ObjectIndices.AddZeroed( UObject::GObjObjects.Num() );
		NameIndices  .AddZeroed( FName::GetMaxNames() );

		// Success.
		Success=1;
	}
	void Destroy()
	{
		guard(ULinkerSave::Destroy);
		if( Saver )
			delete Saver;
		Saver = NULL;
		Super::Destroy();
		unguard;
	}

	// FArchive interface.
	INT MapName( FName* Name )
	{
		guardSlow(ULinkerSave::MapName);
		return NameIndices(Name->GetIndex());
		unguardobjSlow;
	}
	INT MapObject( UObject* Object )
	{
		guardSlow(ULinkerSave::MapObject);
		return Object ? ObjectIndices(Object->GetIndex()) : 0;
		unguardobjSlow;
	}

	// FArchive interface.
	FArchive& operator<<( FName& Name )
	{
		guardSlow(ULinkerSave<<FName);
		INT Save = NameIndices(Name.GetIndex());
		return *this << AR_INDEX(Save);
		unguardobjSlow;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		guardSlow(ULinkerSave<<UObject);
		INT Save = Obj ? ObjectIndices(Obj->GetIndex()) : 0;
		return *this << AR_INDEX(Save);
		unguardobjSlow;
	}
	void Seek( INT InPos )
	{
		Saver->Seek( InPos );
	}
	INT Tell()
	{
		return Saver->Tell();
	}
	void Serialize( void* V, INT Length )
	{
		Saver->Serialize( V, Length );
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
