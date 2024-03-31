/*=============================================================================
	UnEdTran.h: Unreal transaction tracking system
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UTransactor.
-----------------------------------------------------------------------------*/

//
// Object responsible for tracking transactions for undo/redo.
//
class EDITOR_API UTransactor : public UObject
{
	DECLARE_ABSTRACT_CLASS(UTransactor,UObject,CLASS_Transient)

	// UTransactor interface.
	virtual void Reset( const TCHAR* Action )=0;
	virtual void Begin( const TCHAR* SessionName )=0;
	virtual void End()=0;
	virtual void Continue()=0;
	virtual UBOOL CanUndo( FString* Str=NULL )=0;
	virtual UBOOL CanRedo( FString* Str=NULL )=0;
	virtual UBOOL Undo()=0;
	virtual UBOOL Redo()=0;
	virtual FTransactionBase* CreateInternalTransaction()=0;
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
