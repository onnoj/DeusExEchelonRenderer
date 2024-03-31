/*=============================================================================
	EditorPrivate.h: Unreal editor public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/

#ifndef EDITOR_API
	#define EDITOR_API DLL_IMPORT
#endif

/*-----------------------------------------------------------------------------
	Advance editor private definitions.
-----------------------------------------------------------------------------*/

//
// Quality level for rebuilding Bsp.
//
enum EBspOptimization
{
	BSP_Lame,
	BSP_Good,
	BSP_Optimal
};

//
// Things to set in mapSetBrush.
//
enum EMapSetBrushFlags				
{
	MSB_BrushColor	= 1,			// Set brush color.
	MSB_Group		= 2,			// Set group.
	MSB_PolyFlags	= 4,			// Set poly flags.
};

//
// Possible positions of a child Bsp node relative to its parent (for BspAddToNode).
//
enum ENodePlace 
{
	NODE_Back		= 0, // Node is in back of parent              -> Bsp[iParent].iBack.
	NODE_Front		= 1, // Node is in front of parent             -> Bsp[iParent].iFront.
	NODE_Plane		= 2, // Node is coplanar with parent           -> Bsp[iParent].iPlane.
	NODE_Root		= 3, // Node is the Bsp root and has no parent -> Bsp[0].
};

/*-----------------------------------------------------------------------------
	Editor public includes.
-----------------------------------------------------------------------------*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	Editor private includes.
-----------------------------------------------------------------------------*/

#include "UnEdTran.h"
#include "UnTopics.h"

EDITOR_API extern class FGlobalTopicTable GTopics;

/*-----------------------------------------------------------------------------
	Factories.
-----------------------------------------------------------------------------*/

class EDITOR_API ULevelFactoryNew : public UFactory
{
	DECLARE_CLASS(ULevelFactoryNew,UFactory,0)
	FStringNoInit LevelTitle;
	FStringNoInit Author;
	BITFIELD CloseExistingWindows;
	ULevelFactoryNew();
	void StaticConstructor();
	void Serialize( FArchive& Ar );
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, FFeedbackContext* Warn );
};

class EDITOR_API UClassFactoryNew : public UFactory
{
	DECLARE_CLASS(UClassFactoryNew,UFactory,0)
	FName ClassName;
	UPackage* ClassPackage;
	UClass* Superclass;
	UClassFactoryNew();
	void StaticConstructor();
	void Serialize( FArchive& Ar );
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, FFeedbackContext* Warn );
};

class EDITOR_API UTextureFactoryNew : public UFactory
{
	DECLARE_CLASS(UTextureFactoryNew,UFactory,0)
	FName TextureName;
	UPackage* TexturePackage;
	UClass* TextureClass;
	INT USize;
	INT VSize;
	UTextureFactoryNew();
	void StaticConstructor();
	void Serialize( FArchive& Ar );
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, FFeedbackContext* Warn );
};

class EDITOR_API UClassFactoryUC : public UFactory
{
	DECLARE_CLASS(UClassFactoryUC,UFactory,0)
	UClassFactoryUC();
	void StaticConstructor();
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API ULevelFactory : public UFactory
{
	DECLARE_CLASS(ULevelFactory,UFactory,0)
	ULevelFactory();
	void StaticConstructor();
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UPolysFactory : public UFactory
{
	DECLARE_CLASS(UPolysFactory,UFactory,0)
	UPolysFactory();
	void StaticConstructor();
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UModelFactory : public UFactory
{
	DECLARE_CLASS(UModelFactory,UFactory,0)
	UModelFactory();
	void StaticConstructor();
	UObject* FactoryCreateText( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API USoundFactory : public UFactory
{
	DECLARE_CLASS(USoundFactory,UFactory,0)
	USoundFactory();
	void StaticConstructor();
	UObject* FactoryCreateBinary( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const BYTE*& Buffer, const BYTE* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UMusicFactory : public UFactory
{
	DECLARE_CLASS(UMusicFactory,UFactory,0)
	UMusicFactory();
	void StaticConstructor();
	UObject* FactoryCreateBinary( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const BYTE*& Buffer, const BYTE* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UTextureFactory : public UFactory
{
	DECLARE_CLASS(UTextureFactory,UFactory,0)
	UTextureFactory();
	void StaticConstructor();
	UObject* FactoryCreateBinary( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const BYTE*& Buffer, const BYTE* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UFontFactory : public UTextureFactory
{
	DECLARE_CLASS(UFontFactory,UTextureFactory,0)
	UFontFactory();
	void StaticConstructor();
	UObject* FactoryCreateBinary( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, const TCHAR* Type, const BYTE*& Buffer, const BYTE* BufferEnd, FFeedbackContext* Warn );
};

class EDITOR_API UTrueTypeFontFactory : public UFontFactory
{
	DECLARE_CLASS(UTrueTypeFontFactory,UFontFactory,0)
	FStringNoInit	FontName;
	INT				Height;
	INT				USize;
	INT				VSize;
	INT				XPad;
	INT				YPad;
	INT				CharactersPerPage;
	INT				Count;
	FLOAT			Gamma;
	FStringNoInit	Chars;
	UBOOL			AntiAlias;
	FString			List;
	UTrueTypeFontFactory();
	void StaticConstructor();
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, DWORD Flags, UObject* Context, FFeedbackContext* Warn );
};

/*-----------------------------------------------------------------------------
	Exporters.
-----------------------------------------------------------------------------*/

class EDITOR_API UTextBufferExporterTXT : public UExporter
{
	DECLARE_CLASS(UTextBufferExporterTXT,UExporter,0)
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Out, FFeedbackContext* Warn );
};

class EDITOR_API USoundExporterWAV : public UExporter
{
	DECLARE_CLASS(USoundExporterWAV,UExporter,0)
	void StaticConstructor();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UMusicExporterTracker : public UExporter
{
	DECLARE_CLASS(UMusicExporterTracker,UExporter,0)
	void StaticConstructor();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UClassExporterH : public UExporter
{
	DECLARE_CLASS(UClassExporterH,UExporter,0)
	UBOOL DidTop;
	INT RecursionDepth;
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Out, FFeedbackContext* Warn );
};

class EDITOR_API UClassExporterUC : public UExporter
{
	DECLARE_CLASS(UClassExporterUC,UExporter,0)
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UPolysExporterT3D : public UExporter
{
	DECLARE_CLASS(UPolysExporterT3D,UExporter,0)
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UModelExporterT3D : public UExporter
{
	DECLARE_CLASS(UModelExporterT3D,UExporter,0)
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn );
};

class EDITOR_API ULevelExporterT3D : public UExporter
{
	DECLARE_CLASS(ULevelExporterT3D,UExporter,0)
	void StaticConstructor();
	UBOOL ExportText( UObject* Object, const TCHAR* Type, FOutputDevice& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UTextureExporterPCX : public UExporter
{
	DECLARE_CLASS(UTextureExporterPCX,UExporter,0)
	void StaticConstructor();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn );
};

class EDITOR_API UTextureExporterBMP : public UExporter
{
	DECLARE_CLASS(UTextureExporterBMP,UExporter,0)
	void StaticConstructor();
	UBOOL ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
