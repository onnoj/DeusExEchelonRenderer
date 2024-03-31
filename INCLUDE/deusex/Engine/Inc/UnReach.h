/*=============================================================================
	UnReach.h: AI reach specs.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

class ENGINE_API FReachSpec
{

public:
	INT distance; 
	AActor *Start;
	AActor *End; //actor at endpoint of this path (next waypoint or goal)
	INT CollisionRadius; 
    INT CollisionHeight; 
	INT reachFlags; //see defined bits above
	BYTE  bPruned;

	/*
	supports() -
	 returns true if it supports the requirements of aPawn.  Distance is not considered.
	*/
	inline int supports (int iRadius, int iHeight, int moveFlags)
	{
		return ( (CollisionRadius >= iRadius) 
			&& (CollisionHeight >= iHeight)
			&& ((reachFlags & moveFlags) == reachFlags) );
	}
	FReachSpec operator+ (const FReachSpec &Spec) const;
	int defineFor (AActor * begin, AActor * dest, APawn * Scout);
	int operator<= (const FReachSpec &Spec);
	int operator== (const FReachSpec &Spec);
	int MonsterPath();
	int BotOnlyPath();

	void Init()
	{
		guard(FReachSpec::Init);
		// Init everything here.
		Start = End = NULL;
		distance = CollisionRadius = CollisionHeight = 0;
		reachFlags = 0;
		bPruned = 0;
		unguard;
	};

	friend FArchive& operator<< (FArchive &Ar, FReachSpec &ReachSpec )
	{
		guard(FReachSpec<<);
		Ar << ReachSpec.distance << ReachSpec.Start << ReachSpec.End;
		Ar << ReachSpec.CollisionRadius << ReachSpec.CollisionHeight;
		Ar << ReachSpec.reachFlags << ReachSpec.bPruned;
		return Ar;
		unguard;
	};

	// DEUS_EX STM - changed
//	int findBestReachable(FVector &Start, FVector &Destination, APawn * Scout);
	int findBestReachable(FVector &Start, FVector &Destination, FLOAT &StartOffset, FLOAT &DestOffset, APawn * Scout);
};

