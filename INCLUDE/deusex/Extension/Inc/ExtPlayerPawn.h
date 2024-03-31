
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtPlayerPawn.h
//  Programmer  :  Scott Martin
//  Description :  Header for the PlayerPawn extended class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_PLAYER_PAWN_H_
#define _EXT_PLAYER_PAWN_H_


// ----------------------------------------------------------------------

class XRootWindow;


// ----------------------------------------------------------------------
// Structure used for actor references (PRIVATE!)

struct XActorRef
{
	friend class APlayerPawnExt;
	private:
		AActor *actor;
		INT    refCount;
};

const INT MAX_ACTOR_REF=32;


// ----------------------------------------------------------------------
// APlayerPawnExt class

class EXTENSION_API APlayerPawnExt : public APlayerPawn
{
	DECLARE_CLASS(APlayerPawnExt, APlayerPawn, 0)

	public:
		APlayerPawnExt();

		void Destroy(void);

		XFlagBase   *flagBase;     // Knowledge base for the player

		XRootWindow *rootWindow;   // Root window

	private:
		XActorRef actorList[MAX_ACTOR_REF];  // List of valid actors
		INT       actorCount;                // Count of valid actors

	public:
		// APlayerPawnExt interface
		virtual void PreRenderWindows(UCanvas *canvas);
		virtual void PostRenderWindows(UCanvas *canvas);

		void  InitRootWindow(void);

		UBOOL SetMouseDelta(FLOAT dX, FLOAT dY);
		UBOOL SetMousePosition(FLOAT newX, FLOAT newY);

		// Misc. routines
		void  AddActorRef(AActor *actor);
		void  RemoveActorRef(AActor *actor);
		UBOOL IsActorValid(AActor *actor);

		void ConstructRootWindow(void);

	private:
		INT  FindActor(AActor *actor);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execPreRenderWindows)
		DECLARE_FUNCTION(execPostRenderWindows)
		DECLARE_FUNCTION(execInitRootWindow)

};  // APlayerPawnExt


#endif // _EXT_PLAYER_PAWN_H_
