/*=============================================================================
	APawn.h: Class functions residing in the APawn class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

	// Also defined in UnPath.h
	enum EReachSpecFlags
	{
		R_WALK = 1,	//walking required
		R_FLY = 2,   //flying required 
		R_SWIM = 4,  //swimming required
		R_JUMP = 8,   // jumping required
		R_DOOR = 16,
		R_SPECIAL = 32,
		R_PLAYERONLY = 64
	}; 

	// Constructors.
	APawn() {}

	// AActor interface.
	FLOAT GetNetPriority( AActor* Sent, FLOAT Time, FLOAT Lag );
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map );

	// Latent movement
	void setMoveTimer(FLOAT MoveSize);
	int moveToward(const FVector &Dest);
	int rotateToward(const FVector &FocalPoint);
	int PickWallAdjust();

	// Seeing and hearing checks
	void ShowSelf();
	int CanHear(FVector NoiseLoc, FLOAT Loudness, APawn *Other); 
	DWORD LineOfSightTo(AActor *Other, int bShowSelf = 0, bool bIgnoreDistance = false);
	void CheckEnemyVisible();

	int walkToward(const FVector &Destination, FLOAT Movesize);

	// reach tests
	int pointReachable(FVector aPoint, int bKnowVisible = 0);
	int actorReachable(AActor *Other, int bKnowVisible = 0);
	int Reachable(FVector aPoint, FLOAT Threshold, AActor* GoalActor);
	int walkReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor);
	int jumpReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor);
	int flyReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor);
	int swimReachable(FVector Dest, FLOAT Threshold, int reachFlags, AActor* GoalActor);
	void jumpLanding(FVector testvel, FVector &Landing, int moveActor = 0);
	int walkMove(FVector Delta, FCheckResult& Hit, AActor* GoalActor = NULL, FLOAT threshold = 4.1, int bAdjust = 1);
	int flyMove(FVector Delta, AActor* GoalActor, FLOAT threshold = 4.1, int bAdjust = 1);
	int swimMove(FVector Delta, AActor* GoalActor, FLOAT threshold = 4.1, int bAdjust = 1);
	int FindBestJump(FVector Dest, FVector vel, FVector &Landing, int moveActor = 0); 
	int FindJumpUp(FVector Dest, FVector vel, FVector &Landing, int moveActor = 0); 
	void SuggestJumpVelocity(FVector Dest, FVector &Vel);

	// Path finding
	int findPathTo(FVector Dest, INT bSinglePath, AActor *&bestPath, INT bClearPaths);
	int findPathToward(AActor *goal, INT bSinglePath, AActor *&bestPath, INT bClearPaths);
	FLOAT findPathTowardBestInventory(AActor *&bestPath, INT bClearPaths, FLOAT MinWeight, INT bPredictRespawns);
	int findRandomDest(AActor *&bestPath);
	int TraverseFrom(AActor *startnode, int moveFlags);
	int breadthPathFrom(AActor *startnode, AActor *&bestPath, int bSinglePath, int moveFlags);
	FLOAT breadthPathToInventory(AActor *startnode, AActor *&bestPath, int moveFlags, FLOAT bestInventoryWeight, INT bPredictRespawns);
	inline int calcMoveFlags()
	{
		guard(APawn::calcMoveFlags);
		return ( bCanWalk * R_WALK + bCanFly * R_FLY + bCanSwim * R_SWIM + bCanJump * R_JUMP 
				+ bCanOpenDoors * R_DOOR + bCanDoSpecial * R_SPECIAL + bIsPlayer * R_PLAYERONLY); 
		unguard;
	}
	void clearPaths();
	void clearPath(ANavigationPoint *node);
	void HandleSpecial(AActor *&bestPath);
	int CanMoveTo(AActor *Anchor, AActor *Dest);
	void SetRouteCache(ANavigationPoint *BestPath);
	void ReverseRouteFor(ANavigationPoint *BestDest);

	// Pawn physics modes
	void physWalking(FLOAT deltaTime, INT Iterations);
	void physFlying(FLOAT deltaTime, INT Iterations);
	void physSwimming(FLOAT deltaTime, INT Iterations);
	void physSpider(FLOAT deltaTime, INT Iterations);
	void startSwimming(FVector OldVelocity, FLOAT timeTick, FLOAT remainingTime, INT Iterations);
	void physicsRotation(FLOAT deltaTime, FVector OldVelocity);
	void performPhysics(FLOAT DeltaSeconds);

	// Additional AI functions - DEUS_EX STM
	FLOAT AICanSee(AActor *Other, FLOAT visibility=1.0,
	               UBOOL bCheckVisibility=true, UBOOL bCheckDir=true,
	               UBOOL bCheckCylinder=false, UBOOL bCheckLOS=true);
	FLOAT AICanHear(AActor *Other, FLOAT volume=1.0, FLOAT radius=-1.0);
	FLOAT AICanSmell(AActor *Other, FLOAT smell=1.0);
	UBOOL AIDirectionReachable(FVector focus, INT yaw, INT pitch, FLOAT minDist, FLOAT maxDist,
	                           FLOAT threshold=15.0, FVector *pBestDest=NULL);
	UBOOL AIPickRandomDestination(FLOAT minDist, FLOAT maxDist,
	                              INT centralYaw=0, FLOAT yawDistribution=0.0,
	                              INT centralPitch=0, FLOAT pitchDistribution=0.0,
	                              INT tries=1, FLOAT multiplier=1.0,
	                              FVector *pDest=NULL);

	// Natives.
	DECLARE_FUNCTION(execPollWaitForLanding)
	DECLARE_FUNCTION(execPollMoveTo)
	DECLARE_FUNCTION(execPollMoveToward)
	DECLARE_FUNCTION(execPollStrafeTo)
	DECLARE_FUNCTION(execPollStrafeFacing)
	DECLARE_FUNCTION(execPollTurnToward)
	DECLARE_FUNCTION(execPollTurnTo)

private:
	FLOAT Swim(FVector Delta, FCheckResult &Hit); 
	void findWaterLine(FVector Start, FVector &End);
	void stepUp(FVector GravDir, FVector DesiredDir, FVector Delta, FCheckResult &Hit);
	void calcVelocity(FVector AccelDir, FLOAT deltaTime, FLOAT maxSpeed, FLOAT friction, INT bFluid, INT bBrake, INT bBuoyant);
	int findNewFloor(FVector OldLocation, FLOAT deltaTime, FLOAT remainingTime, INT Iterations);
	int checkFloor(FVector Dir, FCheckResult &Hit);

	// Additional private functions -- DEUS_EX STM
	INT GetPathnodeList(struct _FNavInfo *points, INT maxCount,
	                    AActor *fromActor, UBOOL bUsePrunedPaths, UBOOL bSort);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
