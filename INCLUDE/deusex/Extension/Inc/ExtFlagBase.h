// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtFlagBase.h
//  Programmer  :  Scott Martin and Albert Yarusso
//  Description :  Header file for Unreal knowledge base
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_FLAG_BASE_H_
#define _EXT_FLAG_BASE_H_


// ----------------------------------------------------------------------
// Hash information

// NOTE: If FLAG_HASH_SIZE changes, FlagBase.uc must also change!

#define FLAG_HASH_SIZE    (64)  // Must be a power of 2
#define FLAG_HASH_MASK    (FLAG_HASH_SIZE-1)


// ----------------------------------------------------------------------
// Internal iterator structure (PRIVATE! Do Not Use!)

struct XFlagIteratorStruct
{
	friend class XFlagBase;
	private:
		EFlagType flagType;
		XFlag     *curFlag;
		INT       hash;
};


// ----------------------------------------------------------------------
// Public iterator type

typedef INT XFlagIterator;


// ----------------------------------------------------------------------
// XFlagBase class

class EXTENSION_API XFlagBase : public XExtensionObject
{
	DECLARE_CLASS(XFlagBase, XExtensionObject, 0)

	public:
		XFlagBase();

		void Destroy(void);

	private:
		XFlag *flagHashTable[FLAG_HASH_SIZE];  // Hash table containing all flags
		INT    defaultFlagExpiration;          // Default flag expiration

	public:
		// Direct flag-manipulation routines
		void   AddFlag(XFlag *newFlag);
		void   RemoveFlag(XFlag *newFlag);
		XFlag *GetFlag(FName flagName, BYTE flagType);

	private:
		void   CheckName(FName &flagName);
		XFlag *FindName(FName &flagName, BYTE flagType, XFlag **&ppPrevFlag);
		UBOOL  FindFlag(XFlag *pFlag, XFlag **&pPrevFlag);

	public:
		// Flag-setting routines
		UBOOL SetBool(FName flagName,    UBOOL    newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetByte(FName flagName,    BYTE     newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetInt(FName flagName,     INT      newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetFloat(FName flagName,   FLOAT    newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetName(FName flagName,    FName    newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetVector(FName flagName,  FVector  newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);
		UBOOL SetRotator(FName flagName, FRotator newValue,  UBOOL bAdd=TRUE,  INT expiration=-1);

		// Flag-retrieving routines
		UBOOL GetBool(FName flagName,    UBOOL    &value);
		UBOOL GetByte(FName flagName,    BYTE     &value);
		UBOOL GetInt(FName flagName,     INT      &value);
		UBOOL GetFloat(FName flagName,   FLOAT    &value);
		UBOOL GetName(FName flagName,    FName    &value);
		UBOOL GetVector(FName flagName,  FVector  &value);
		UBOOL GetRotator(FName flagName, FRotator &value);

		UBOOL CheckFlag(FName flagName, BYTE flagType);

		UBOOL DeleteFlag(FName flagName, BYTE flagType);
		void  DeleteAllFlags(void);

		// Flag Expiration routines
		void  SetExpiration(FName flagName, BYTE flagType, INT expiration);
		INT   GetExpiration(FName flagName, BYTE flagType);
		void  DeleteExpiredFlags(INT criteria);
		void  SetDefaultExpiration(INT expiration);

		// Iterator routines
		XFlagIterator CreateIterator(EFlagType flagType=MAX_FLAG_TYPE);
		UBOOL         GetNextFlag(XFlagIterator iterator,
		                          FName *pName=NULL, EFlagType *pFlagType=NULL);
		void          DestroyIterator(XFlagIterator iterator);

		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetBool)
		DECLARE_FUNCTION(execSetByte)
		DECLARE_FUNCTION(execSetInt)
		DECLARE_FUNCTION(execSetFloat)
		DECLARE_FUNCTION(execSetName)
		DECLARE_FUNCTION(execSetVector)
		DECLARE_FUNCTION(execSetRotator)

		DECLARE_FUNCTION(execGetBool)
		DECLARE_FUNCTION(execGetByte)
		DECLARE_FUNCTION(execGetInt)
		DECLARE_FUNCTION(execGetFloat)
		DECLARE_FUNCTION(execGetName)
		DECLARE_FUNCTION(execGetVector)
		DECLARE_FUNCTION(execGetRotator)

		DECLARE_FUNCTION(execCheckFlag)
		DECLARE_FUNCTION(execDeleteFlag)

		DECLARE_FUNCTION(execSetExpiration)
		DECLARE_FUNCTION(execGetExpiration)
		DECLARE_FUNCTION(execDeleteExpiredFlags)
		DECLARE_FUNCTION(execSetDefaultExpiration)

		DECLARE_FUNCTION(execCreateIterator)
		DECLARE_FUNCTION(execGetNextFlagName)
		DECLARE_FUNCTION(execGetNextFlag)
		DECLARE_FUNCTION(execDestroyIterator)

		DECLARE_FUNCTION(execDeleteAllFlags)


};  // XFlagBase


#endif // _EXT_FLAG_BASE_H_
