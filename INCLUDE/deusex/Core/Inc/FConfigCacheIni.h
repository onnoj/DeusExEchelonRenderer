/*=============================================================================
	FConfigCacheIni.h: Unreal config file reading/writing.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Config cache.
-----------------------------------------------------------------------------*/

// One section in a config file.
class FConfigSection : public TMultiMap<FString,FString>
{};

// One config file.
class FConfigFile : public TMap<FString,FConfigSection>
{
public:
	UBOOL Dirty, NoSave;
	FConfigFile()
	: Dirty( 0 )
	, NoSave( 0 )
	{}
	void Read( const TCHAR* Filename )
	{
		guard(FConfigFile::Read);
		Empty();
		FString Text;
		if( appLoadFileToString( Text, Filename ) )
		{
			TCHAR* Ptr = const_cast<TCHAR*>( *Text );
			FConfigSection* CurrentSection = NULL;
			UBOOL Done = 0;
			while( !Done )
			{
				while( *Ptr=='\r' || *Ptr=='\n' )
					Ptr++;
				TCHAR* Start = Ptr;
				while( *Ptr && *Ptr!='\r' && *Ptr!='\n' )
					Ptr++;
				if( *Ptr==0 )
					Done = 1;
				*Ptr++ = 0;
				if( *Start=='[' && Start[appStrlen(Start)-1]==']' )
				{
					Start++;
					Start[appStrlen(Start)-1] = 0;
					CurrentSection = Find( Start );
					if( !CurrentSection )
						CurrentSection = &Set( Start, FConfigSection() );
				}
				else if( CurrentSection && *Start )
				{
					TCHAR* Value = appStrchr(Start,'=');
					if( Value )
					{
						*Value++ = 0;
						if( *Value=='\"' && Value[appStrlen(Value)-1]=='\"' )
						{
							Value++;
							Value[appStrlen(Value)-1]=0;
						}
						CurrentSection->Add( Start, Value );
					}
				}
			}
		}
		unguard;
	}
	UBOOL Write( const TCHAR* Filename )
	{
		guard(FConfigFile::Write);
		if( !Dirty || NoSave )
			return 1;
		Dirty = 0;
		FString Text;
		for( TIterator It(*this); It; ++It )
		{
			Text += FString::Printf( TEXT("[%s]\r\n"), *It.Key() );
			for( FConfigSection::TIterator It2(It.Value()); It2; ++It2 )
				Text += FString::Printf( TEXT("%s=%s\r\n"), *It2.Key(), *It2.Value() );
			Text += FString::Printf( TEXT("\r\n") );
		}
		return appSaveStringToFile( Text, Filename );
		unguard;
	}
};

// Set of all cached config files.
class FConfigCacheIni : public FConfigCache, public TMap<FString,FConfigFile>
{
public:
	// Basic functions.
	FString SystemIni, UserIni;
	FConfigCacheIni()
	{}
	~FConfigCacheIni()
	{
		guard(FConfigCacheIni::~FConfigCacheIni);
		Flush( 1 );
		unguard;
	}
	FConfigFile* Find( const TCHAR* InFilename, UBOOL CreateIfNotFound )
	{
		guard(FConfigCacheIni::Find);

		// If filename not specified, use default.
		TCHAR Filename[256];
		appStrcpy( Filename, InFilename ? InFilename : *SystemIni );

		// Add .ini extension.
		INT Len = appStrlen(Filename);
		if( Len<5 || (Filename[Len-4]!='.' && Filename[Len-5]!='.') )
			appStrcat( Filename, TEXT(".ini") );

		// Automatically translate generic filenames.
		if( appStricmp(Filename,TEXT("User.ini"))==0 )
			appStrcpy( Filename, *UserIni );
		else if( appStricmp(Filename,TEXT("System.ini"))==0 )
			appStrcpy(Filename,*SystemIni);

		// Get file.
		FConfigFile* Result = TMap<FString,FConfigFile>::Find( Filename );
		if( !Result && (CreateIfNotFound || GFileManager->FileSize(Filename)>=0)  )
		{
			Result = &Set( Filename, FConfigFile() );
			Result->Read( Filename );
		}
		return Result;

		unguard;
	}
	void Flush( UBOOL Read, const TCHAR* Filename=NULL )
	{
		guard(FConfigCacheIni::Flush);
		for( TIterator It(*this); It; ++It )
			if( !Filename || It.Key()==Filename )
				It.Value().Write( *It.Key() );
		if( Read )
		{
			if( Filename )
				Remove(Filename);
			else
				Empty();
		}
		unguard;
	}
	void Detach( const TCHAR* Filename )
	{
		guard(FConfigCacheIni::Cancel);
		FConfigFile* File = Find( Filename, 1 );
		if( File )
			File->NoSave = 1;
		unguard;
	}
	UBOOL GetString( const TCHAR* Section, const TCHAR* Key, TCHAR* Value, INT Size, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::GetString);
		*Value = 0;
		FConfigFile* File = Find( Filename, 0 );
		if( !File )
			return 0;
		FConfigSection* Sec = File->Find( Section );
		if( !Sec )
			return 0;
		FString* PairString = Sec->Find( Key );
		if( !PairString )
			return 0;
		appStrncpy( Value, **PairString, Size );
		return 1;
		unguard;
	}
	UBOOL GetSection( const TCHAR* Section, TCHAR* Result, INT Size, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::GetSection);
		*Result = 0;
		FConfigFile* File = Find( Filename, 0 );
		if( !File )
			return 0;
		FConfigSection* Sec = File->Find( Section );
		if( !Sec )
			return 0;
		TCHAR* End = Result;
		for( FConfigSection::TIterator It(*Sec); It && End-Result+appStrlen(*It.Key())+1<Size; ++It )
			End += appSprintf( End, TEXT("%s=%s"), *It.Key(), *It.Value() ) + 1;
		*End++ = 0;
		return 1;
		unguard;
	}
	TMultiMap<FString,FString>* GetSectionPrivate( const TCHAR* Section, UBOOL Force, UBOOL Const, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::GetSectionPrivate);
		FConfigFile* File = Find( Filename, Force );
		if( !File )
			return NULL;
		FConfigSection* Sec = File->Find( Section );
		if( !Sec && Force )
			Sec = &File->Set( Section, FConfigSection() );
		if( Sec && (Force || !Const) )
			File->Dirty = 1;
		return Sec;
		unguard;
	}
	void SetString( const TCHAR* Section, const TCHAR* Key, const TCHAR* Value, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::SetString);
		FConfigFile* File = Find( Filename, 1 );
		FConfigSection* Sec  = File->Find( Section );
		if( !Sec )
			Sec = &File->Set( Section, FConfigSection() );
		FString* Str = Sec->Find( Key );
		if( !Str )
		{
			Sec->Add( Key, Value );
			File->Dirty = 1;
		}
		else if( appStricmp(**Str,Value)!=0 )
		{
			File->Dirty = (appStrcmp(**Str,Value)!=0);
			*Str = Value;
		}
		unguard;
	}
	void EmptySection( const TCHAR* Section, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::EmptySection);
		FConfigFile* File = Find( Filename, 0 );
		if( File )
		{
			FConfigSection* Sec = File->Find( Section );
			if( Sec && FConfigSection::TIterator(*Sec) )
			{
				Sec->Empty();
				File->Dirty = 1;
			}
		}
		unguard;
	}
	void Init( const TCHAR* InSystem, const TCHAR* InUser, UBOOL RequireConfig )
	{
		guard(FConfigCacheIni::Init);
		SystemIni = InSystem;
		UserIni   = InUser;
		unguard;
	}
	void Exit()
	{
		guard(FConfigCacheIni::Exit);
		Flush( 1 );
		unguard;
	}
	void Dump( FOutputDevice& Ar )
	{
		guard(FConfigCacheIni::Dump);
		Ar.Log( TEXT("Files map:") );
		TMap<FString,FConfigFile>::Dump( Ar );
		unguard;
	}

	// Derived functions.
	UBOOL GetString
	(
		const TCHAR* Section,
		const TCHAR* Key,
		FString&     Str,
		const TCHAR* Filename
	)
	{
		guard(FConfigCacheIni::GetString);
		TCHAR Temp[4096]=TEXT("");
		UBOOL Result = GetString( Section, Key, Temp, ARRAY_COUNT(Temp), Filename );
		Str = Temp;
		return Result;
		unguard;
	}
	const TCHAR* GetStr( const TCHAR* Section, const TCHAR* Key, const TCHAR* Filename )
	{
		guard(FConfigCacheIni::GetStr);
		TCHAR* Result = appStaticString1024();
		GetString( Section, Key, Result, 1024, Filename );
		return Result;
		unguard;
	}
	UBOOL GetInt
	(
		const TCHAR*	Section,
		const TCHAR*	Key,
		INT&			Value,
		const TCHAR*	Filename
	)
	{
		guard(FConfigCacheIni::GetInt);
		TCHAR Text[80]; 
		if( GetString( Section, Key, Text, ARRAY_COUNT(Text), Filename ) )
		{
			Value = appAtoi(Text);
			return 1;
		}
		return 0;
		unguard;
	}
	UBOOL GetFloat
	(
		const TCHAR*	Section,
		const TCHAR*	Key,
		FLOAT&			Value,
		const TCHAR*	Filename
	)
	{
		guard(FConfigCacheIni::GetFloat);
		TCHAR Text[80]; 
		if( GetString( Section, Key, Text, ARRAY_COUNT(Text), Filename ) )
		{
			Value = appAtof(Text);
			return 1;
		}
		return 0;
		unguard;
	}
	UBOOL GetBool
	(
		const TCHAR*	Section,
		const TCHAR*	Key,
		UBOOL&			Value,
		const TCHAR*	Filename
	)
	{
		guard(FConfigCacheIni::GetBool);
		TCHAR Text[80]; 
		if( GetString( Section, Key, Text, ARRAY_COUNT(Text), Filename ) )
		{
			if( appStricmp(Text,TEXT("True"))==0 )
			{
				Value = 1;
			}
			else
			{
				Value = appAtoi(Text)==1;
			}
			return 1;
		}
		return 0;
		unguard;
	}
	void SetInt
	(
		const TCHAR* Section,
		const TCHAR* Key,
		INT			 Value,
		const TCHAR* Filename
	)
	{
		guard(FConfigCacheIni::SetInt);
		TCHAR Text[30];
		appSprintf( Text, TEXT("%i"), Value );
		SetString( Section, Key, Text, Filename );
		unguard;
	}
	void SetFloat
	(
		const TCHAR*	Section,
		const TCHAR*	Key,
		FLOAT			Value,
		const TCHAR*	Filename
	)
	{
		guard(FConfigCacheIni::SetFloat);
		TCHAR Text[30];
		appSprintf( Text, TEXT("%f"), Value );
		SetString( Section, Key, Text, Filename );
		unguard;
	}
	void SetBool
	(
		const TCHAR* Section,
		const TCHAR* Key,
		UBOOL		 Value,
		const TCHAR* Filename
	)
	{
		guard(FConfigCacheIni::SetBool);
		SetString( Section, Key, Value ? TEXT("True") : TEXT("False"), Filename );
		unguard;
	}

	// Static allocator.
	static FConfigCache* Factory()
	{
		return new FConfigCacheIni();
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
