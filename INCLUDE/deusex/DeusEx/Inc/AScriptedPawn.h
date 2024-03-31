// ----------------------------------------------------------------------
//  File Name   :  AScriptedPawn.h
//  Programmer  :  Albert Yarusso
//  Description :  Additional declarations for ScriptedPawn
// ----------------------------------------------------------------------
//  Copyright ©1998 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

// Eight lines of comments and two lines of code...  :)  [not any more! -STM]
	NO_DEFAULT_CONSTRUCTOR(AScriptedPawn)

	void ConBindEvents(void);
	UBOOL IsValidEnemy(APawn *testPawn, UBOOL bCheckAlliance=TRUE);
	EAllianceType GetAllianceType(FName allianceName);
	EAllianceType GetPawnAllianceType(APawn *queryPawn);
	UBOOL Tick(FLOAT deltaSeconds, ELevelTick tickType);
	void UpdateAgitation(FLOAT deltaSeconds);
	void UpdateFear(FLOAT deltaSeconds);
	UBOOL HaveSeenCarcass(FName carcassName);
	void AddCarcass(FName carcassName);
