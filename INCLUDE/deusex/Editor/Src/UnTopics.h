/*=============================================================================
	UnTopics.h: Unreal information topics for Unreal/UnrealEd communication
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-------------------------------------------------------------------------------
	FTopicHandler.
-------------------------------------------------------------------------------*/

//
// The topic handler base class.  All specific topic handlers are derived from this.
//
class FTopicHandler
{
public:
	// Name of topic handler, set by FTopicTable::Register.
	TCHAR TopicName[NAME_SIZE];

	// Next topic handler in linked list of global topic list.
	FTopicHandler* Next;

	// Public functions.
	virtual void Set( ULevel* Level, const TCHAR* Item, const TCHAR* Data )=0;
	virtual void Get( ULevel* Level, const TCHAR* Item, FOutputDevice& Ar )=0;
};

/*-------------------------------------------------------------------------------
	FGlobalTopicTable.
-------------------------------------------------------------------------------*/

//
// Global topic table class.  Contains all available topics.
//
class EDITOR_API FGlobalTopicTable
{
public:
	// Variables.
	FTopicHandler* FirstHandler;

	// Functions.
	void Register( const TCHAR* TopicName, FTopicHandler* Handler );
	void Init();
	void Exit();
	void Get( ULevel* Level, const TCHAR* Topic, const TCHAR* Item, FOutputDevice& Ar );
	void Set( ULevel* Level, const TCHAR* Topic, const TCHAR* Item, const TCHAR* Value );

private:
	FTopicHandler* Find( const TCHAR* TopicName );
};

/*-------------------------------------------------------------------------------
	Autoregistration macro.
-------------------------------------------------------------------------------*/

//
// Register a topic with Unreal's global topic manager (GTopics).  This
// macro should be used exactly once at global scope for every unique topic
// that must be made available.
//
// The macro creates a bogus global variable and assigns it a value returned from
// the topic class's *Name constructor, which has the effect of
// registering the topic primordially, before main() has been called.
//
#define AUTOREGISTER_TOPIC(NAME,HANDLERCLASS) \
class HANDLERCLASS : public FTopicHandler \
{ \
public: \
	HANDLERCLASS( TCHAR* Name ) { GTopics.Register( Name, this ); } \
	void Get( ULevel* Level, const TCHAR* Item, FOutputDevice& Ar ); \
	void Set( ULevel* Level, const TCHAR* Item, const TCHAR* Value ); \
} autoregister##HANDLERCLASS( NAME );

/*-------------------------------------------------------------------------------
	The End.
-------------------------------------------------------------------------------*/
