
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtViewport.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal viewport widgets
// ----------------------------------------------------------------------
//  Copyright �1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_VIEWPORT_H_
#define _EXT_VIEWPORT_H_


// ----------------------------------------------------------------------
// XViewportWindow class

class EXTENSION_API XViewportWindow : public XWindow
{
	DECLARE_CLASS(XViewportWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XViewportWindow)

	public:
		XViewportWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

		BITFIELD     bEnableViewport:1 GCC_PACK(4);
		BITFIELD     bClearZ:1;
		BITFIELD     bShowActor:1;
		BITFIELD     bShowWeapons:1;
		BITFIELD     bUseViewRotation:1;
		BITFIELD     bUseEyeHeight:1;
		BITFIELD     bWatchEyeHeight:1;

		FLOAT        fov GCC_PACK(4);

		UTexture     *defaultTexture;
		FColor       defaultColor;

		class AActor *originActor;
		class AActor *watchActor;
		FVector      location;
		FVector      relLocation;
		FRotator     rotation;
		FRotator     relRotation;

	private:
		BITFIELD     bOriginActorDestroyed:1 GCC_PACK(4);
		FVector      lastLocation GCC_PACK(4);
		FRotator     lastRotation;

		struct FSceneNode *viewportFrame;

	public:
		// XViewportWindow interface

		// Where you're looking from (mutually exclusive)
		void SetViewportActor(class AActor *newOriginActor=NULL,
		                      UBOOL bEyeLevel=TRUE, UBOOL bEnable=TRUE);
		void SetViewportLocation(FVector newLocation=FVector(0, 0, 0),
		                         UBOOL bEnable=TRUE);

		// Where you're looking toward (mutually exclusive)
		void SetWatchActor(class AActor *newWatchActor=NULL,
		                   UBOOL bEyeLevel=TRUE);
		void SetRotation(FRotator newRotation=FRotator(0, 0, 0));

		// Additional methods
		void EnableViewport(UBOOL bEnabled=TRUE);
		void SetFOVAngle(FLOAT newAngle=90);
		void ShowViewportActor(UBOOL bNewShowActor=TRUE);
		void ShowWeapons(UBOOL bNewShowWeapons=TRUE);

		void SetRelativeLocation(FVector relLocation=FVector(0, 0, 0));
		void SetRelativeRotation(FRotator relRotation=FRotator(0, 0, 0));

		void SetDefaultTexture(UTexture *newTexture=NULL,
		                       FColor newColor=FColor(255, 255, 255));
		void ClearZBuffer(UBOOL bClear=TRUE);

		// XViewportWindow interface callbacks
		virtual void CalcView(class AActor *originActor, class AActor *watchActor,
		                      FVector &frameLocation, FRotator &frameRotation);

		// XWindow interface callbacks
		void Draw(XGC *gc);
		void PostDraw(XGC *gc);

		UBOOL ConvertVectorToCoordinates(FVector location,
		                                 FLOAT *pRelativeX=NULL, FLOAT *pRelativeY=NULL);

	protected:
		virtual void RenderFrame(class URenderBase *render, struct FSceneNode *frame);

	private:
		class URenderBase *GetRenderer(void);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execSetViewportActor)
		DECLARE_FUNCTION(execSetViewportLocation)

		DECLARE_FUNCTION(execSetWatchActor)
		DECLARE_FUNCTION(execSetRotation)

		DECLARE_FUNCTION(execEnableViewport)
		DECLARE_FUNCTION(execSetFOVAngle)
		DECLARE_FUNCTION(execShowViewportActor)
		DECLARE_FUNCTION(execShowWeapons)

		DECLARE_FUNCTION(execSetRelativeLocation)
		DECLARE_FUNCTION(execSetRelativeRotation)

		DECLARE_FUNCTION(execSetDefaultTexture)
		DECLARE_FUNCTION(execClearZBuffer)

};  // XViewportWindow


#endif // _EXT_VIEWPORT_H_
