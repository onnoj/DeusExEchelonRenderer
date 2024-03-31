// ----------------------------------------------------------------------
//  File Name   :  ConCamera.h
//  Programmer  :  Albert Yarusso (ay)
//  Description :  Header for Conversation Camera class
//
// ----------------------------------------------------------------------
//  Copyright �1998 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _CONCAMERA_H_
#define _CONCAMERA_H_

// ----------------------------------------------------------------------
// DConCamera Class
// 
// This class is used to pass camera information to 
// DeusExPlayer::PlayerCalcView() during conversations to position the
// camera in different locations.
// ----------------------------------------------------------------------

class CONSYS_API DConCamera : public DConObject
{
	DECLARE_CLASS(DConCamera, DConObject, 0)

public:
	class AActor* cameraActor;			// Actor who owns this event
	BYTE  cameraPosition;				// Predefined camera position
	BYTE  cameraType;					// Camera Type for current event
	BYTE  cameraMode;					// Current camera display mode

	class DConLight* conLightSpeaker;      //Hanfling: Used to light actor's faces (WAS MISSING)
	class DConLight* conLightSpeakingTo;   //Hanfling: Used to light actor's faces (WAS MISSING)
   
	FVector cameraOffset;				// Camera offset, for CT_Actor mode
	FRotator rotation;					// Camera Rotation
	
	FLOAT cosAngle; 					//Hanfling: Was missing.
	INT firstActorRotation; 			//Hanfling: Was missing.
	INT setActorCount;					//Hanfling: Was missing.
	BITFIELD bCameraLocationSaved:1;	//Hanfling: Was missing.

	// Camera Fallback Positions (for when camera view obstructed)
	BYTE cameraFallbackPositions[9]; 	//Hanfling: ECameraPositions (WAS MISSING)
	BYTE cameraHeightPositions[9]; 		//Hanfling: ECameraPositions (WAS MISSING)

	INT currentFallback; 				//Hanfling: Was missing.
	UBOOL bUsingFallback; 				//Hanfling: Was missing.
	
	// Used for CT_Speakers mode
	FLOAT heightModifer;				// Height Modifier
	FLOAT centerModifier;				// Center Point modifier
	FLOAT distanceMultiplier;			// Distance Multiplier

	FLOAT heightFallbackTrigger;		//Hanfling: Was missing.
	
	// Actors associated with camera placement
	class AActor* firstActor;			
	class AActor* secondActor;

	// These variable are used to prevent camera angle changes when the 
	// actors change.
	class AActor* lastFirstActor;
	class AActor* lastSecondActor;
	BITFIELD ignoreSetActors:1;
	
	// Used for Camera debugging
	BITFIELD bDebug:1;
	FVector LastLocation;
	FRotator LastRotation;
	BITFIELD bInteractiveCamera:1;

	// Constructor
	DConCamera();
};

#endif // _CONCAMERA_H_

