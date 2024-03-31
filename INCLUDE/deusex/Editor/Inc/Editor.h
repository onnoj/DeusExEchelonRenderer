/*=============================================================================
	Editor.h: Unreal editor public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_EDITOR
#define _INC_EDITOR

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "Engine.h"

struct EDITOR_API FBuilderPoly
{
	TArray<INT> VertexIndices;
	INT Direction;
	FName ItemName;
	INT PolyFlags;
	FBuilderPoly()
	: VertexIndices(), Direction(0), ItemName(NAME_None)
	{}
};

#include "EditorClasses.h"

/*-----------------------------------------------------------------------------
	Editor public.
-----------------------------------------------------------------------------*/

//
// The editor object.
//
EDITOR_API extern class UEditorEngine* GEditor;

//
// Importing object properties.
//
EDITOR_API const TCHAR* ImportProperties
(
	UClass*				ObjectClass,
	BYTE*				Object,
	ULevel*				Level,
	const TCHAR*		Data,
	UObject*			InParent,
	FFeedbackContext*	Warn
);

//
// Editor mode settings.
//
// These are also referenced by help files and by the editor client, so
// they shouldn't be changed.
//
enum EEditorMode
{
	EM_None 			= 0,	// Gameplay, editor disabled.
	EM_ViewportMove		= 1,	// Move viewport normally.
	EM_ViewportZoom		= 2,	// Move viewport with acceleration.
	EM_BrushRotate		= 5,	// Rotate brush.
	EM_BrushSheer		= 6,	// Sheer brush.
	EM_BrushScale		= 7,	// Scale brush.
	EM_BrushStretch		= 8,	// Stretch brush.
	EM_TexturePan		= 11,	// Pan textures.
	EM_TextureRotate	= 13,	// Rotate textures.
	EM_TextureScale		= 14,	// Scale textures.
	EM_BrushSnap		= 18,	// Brush snap-scale.
	EM_TexView			= 19,	// Viewing textures.
	EM_TexBrowser		= 20,	// Browsing textures.
	EM_MeshView			= 21,	// Viewing mesh.
	EM_MeshBrowser		= 22,	// Browsing mesh.
};

//
// Editor callback codes.
//
enum EUnrealEdCallbacks
{
	EDC_None                = 0,
	EDC_Browse              = 1,
	EDC_UseCurrent			= 2,
	EDC_CurTexChange	    = 10,
	EDC_SelPolyChange	    = 20,
	EDC_SelChange		    = 21,
	EDC_RtClickTexture	    = 23,
	EDC_RtClickPoly		    = 24,
	EDC_RtClickActor	    = 25,
	EDC_RtClickWindow	    = 26,
	EDC_RtClickWindowCanAdd = 27,
	EDC_MapChange		    = 42,
};

//
// Bsp poly alignment types for polyTexAlign.
//
enum ETexAlign						
{
	TEXALIGN_Default		= 0,	// No special alignment (just derive from UV vectors).
	TEXALIGN_Floor			= 1,	// Regular floor (U,V not necessarily axis-aligned).
	TEXALIGN_WallDir		= 2,	// Grade (approximate floor), U,V X-Y axis aligned.
	TEXALIGN_WallPan		= 3,	// Align as wall (V vertical, U horizontal).
	TEXALIGN_OneTile		= 4,	// Align one tile.
	TEXALIGN_WallColumn		= 5,	// Align as wall on column.
};

/*-----------------------------------------------------------------------------
	FEditorHitObserver.
-----------------------------------------------------------------------------*/

//
// Hit observer for editor events.
//
class EDITOR_API FEditorHitObserver : public FHitObserver
{
public:
	// FHitObserver interface.
	void Click( const FHitCause& Cause, const HHitProxy& Hit )
	{
		if     ( Hit.IsA(TEXT("HBspSurf"        )) ) Click( Cause, *(HBspSurf        *)&Hit );
		else if( Hit.IsA(TEXT("HActor"          )) ) Click( Cause, *(HActor          *)&Hit );
		else if( Hit.IsA(TEXT("HBrushVertex"    )) ) Click( Cause, *(HBrushVertex    *)&Hit );
		else if( Hit.IsA(TEXT("HGlobalPivot"    )) ) Click( Cause, *(HGlobalPivot    *)&Hit );
		else if( Hit.IsA(TEXT("HBrowserTexture" )) ) Click( Cause, *(HBrowserTexture *)&Hit );
		else FHitObserver::Click( Cause, Hit );
	}

	// FEditorHitObserver interface.
	virtual void Click( const FHitCause& Cause, const struct HBspSurf&        Hit );
	virtual void Click( const FHitCause& Cause, const struct HActor&          Hit );
	virtual void Click( const FHitCause& Cause, const struct HBrushVertex&    Hit );
	virtual void Click( const FHitCause& Cause, const struct HGlobalPivot&    Hit );
	virtual void Click( const FHitCause& Cause, const struct HBrowserTexture& Hit );
};

/*-----------------------------------------------------------------------------
	Hit proxies.
-----------------------------------------------------------------------------*/

// Hit a texture view.
struct HTextureView : public HHitProxy
{
	DECLARE_HIT_PROXY(HTextureView,HHitProxy)
	UTexture* Texture;
	INT ViewX, ViewY;
	HTextureView( UTexture* InTexture, INT InX, INT InY ) : Texture(InTexture), ViewX(InX), ViewY(InY) {}
	void Click( const FHitCause& Cause );
};

// Hit a brush vertex.
struct HBrushVertex : public HHitProxy
{
	DECLARE_HIT_PROXY(HBrushVertex,HHitProxy)
	ABrush* Brush;
	FVector Location;
	HBrushVertex( ABrush* InBrush, FVector InLocation ) : Brush(InBrush), Location(InLocation) {}
};

// Hit a global pivot.
struct HGlobalPivot : public HHitProxy
{
	DECLARE_HIT_PROXY(HGlobalPivot,HHitProxy)
	FVector Location;
	HGlobalPivot( FVector InLocation ) : Location(InLocation) {}
};

// Hit a browser texture.
struct HBrowserTexture : public HHitProxy
{
	DECLARE_HIT_PROXY(HBrowserTexture,HHitProxy)
	UTexture* Texture;
	HBrowserTexture( UTexture* InTexture ) : Texture(InTexture) {}
};

// Hit the backdrop.
struct HBackdrop : public HHitProxy
{
	DECLARE_HIT_PROXY(HBackdrop,HHitProxy)
	FVector Location;
	HBackdrop( FVector InLocation ) : Location(InLocation) {}
	void Click( const FHitCause& Cause );
};

/*-----------------------------------------------------------------------------
	FScan.
-----------------------------------------------------------------------------*/

typedef void (*POLY_CALLBACK)( UModel* Model, INT iSurf );

/*-----------------------------------------------------------------------------
	FConstraints.
-----------------------------------------------------------------------------*/

//
// General purpose movement/rotation constraints.
//
class EDITOR_API FConstraints
{
public:
	// Functions.
	virtual void Snap( FVector& Point, FVector GridBase )=0;
	virtual void Snap( FRotator& Rotation )=0;
	virtual UBOOL Snap( ULevel* Level, FVector& Location, FVector GridBase, FRotator& Rotation )=0;
};

/*-----------------------------------------------------------------------------
	FConstraints.
-----------------------------------------------------------------------------*/

//
// General purpose movement/rotation constraints.
//
class EDITOR_API FEditorConstraints : public FConstraints
{
public:
	// Variables.
	BITFIELD	GridEnabled:1;		// Grid on/off.
	BITFIELD	SnapVertices:1;		// Snap to nearest vertex within SnapDist, if any.
	FLOAT		SnapDistance;		// Distance to check for snapping.
	FVector		GridSize;			// Movement grid.
	UBOOL		RotGridEnabled;		// Rotation grid on/off.
	FRotator	RotGridSize;		// Rotation grid.

	// Functions.
	virtual void Snap( FVector& Point, FVector GridBase );
	virtual void Snap( FRotator& Rotation );
	virtual UBOOL Snap( ULevel* Level, FVector& Location, FVector GridBase, FRotator& Rotation );
};

/*-----------------------------------------------------------------------------
	UEditorEngine definition.
-----------------------------------------------------------------------------*/

class EDITOR_API UEditorEngine : public UEngine, public FNotifyHook
{
	DECLARE_CLASS(UEditorEngine,UEngine,CLASS_Transient|CLASS_Config)

	// Objects.
	ULevel*					 Level;
	UModel*					 TempModel;
	UTexture*				 CurrentTexture;
	UClass*					 CurrentClass;
	class UTransactor*		 Trans;
	class UTextBuffer*		 Results;
	class WObjectProperties* ActorProperties;
	class WObjectProperties* LevelProperties;
	class WConfigProperties* Preferences;
	class WProperties*       UseDest;
	INT                      AutosaveCounter;
	INT						 Pad[3];

	// Graphics.
	UTexture *MenuUp, *MenuDn;
	UTexture *CollOn, *CollOff;
	UTexture *PlyrOn, *PlyrOff;
	UTexture *LiteOn, *LiteOff;
	UTexture *Bad;
	UTexture *Bkgnd, *BkgndHi;

	// Toggles.
	BITFIELD				FastRebuild  :1;
	BITFIELD				Bootstrapping:1;

	// Variables.
	INT						AutoSaveIndex;
	INT						AutoSaveCount;
	INT						Mode;
	DWORD					ClickFlags;
	FLOAT					MovementSpeed;
	UObject*				ParentContext;
	FVector					ClickLocation;
	FPlane					ClickPlane;

	// Tools.
	TArray<UObject*>		Tools;
	UClass*					BrowseClass;

	// Constraints.
	FEditorConstraints		Constraints;

	// Advanced.
	FLOAT FovAngle;
	BITFIELD GodMode:1;
	BITFIELD AutoSave:1;
	BYTE AutosaveTimeMinutes;
	FStringNoInit GameCommandLine;
	TArray<FString> EditPackages;

	// Color preferences.
	FColor
		C_WorldBox,
		C_GroundPlane,
		C_GroundHighlight,
		C_BrushWire,
		C_Pivot,
		C_Select,
		C_Current,
		C_AddWire,
		C_SubtractWire,
		C_GreyWire,
		C_BrushVertex,
		C_BrushSnap,
		C_Invalid,
		C_ActorWire,
		C_ActorHiWire,
		C_Black,
		C_White,
		C_Mask,
		C_SemiSolidWire,
		C_NonSolidWire,
		C_WireBackground,
		C_WireGridAxis,
		C_ActorArrow,
		C_ScaleBox,
		C_ScaleBoxHi,
		C_ZoneWire,
		C_Mover,
		C_OrthoBackground;

	// Constructor.
	void StaticConstructor();
	UEditorEngine();

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// FNotify interface.
	void NotifyDestroy( void* Src );
	void NotifyPreChange( void* Src );
	void NotifyPostChange( void* Src );
	void NotifyExec( void* Src, const TCHAR* Cmd );

	// UEngine interface.
	void Init();
	void InitEditor();
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	UBOOL HookExec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	int Key( UViewport* Viewport, EInputKey Key );
	void Tick( FLOAT DeltaSeconds );
	void Draw( UViewport* Viewport, UBOOL Blit=1, BYTE* HitData=NULL, INT* HitSize=NULL );
	void MouseDelta( UViewport* Viewport, DWORD Buttons, FLOAT DX, FLOAT DY );
	void MousePosition( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y );
	void Click( UViewport* Viewport, DWORD Buttons, FLOAT X, FLOAT Y );
	void SetClientTravel( UPlayer* Viewport, const TCHAR* NextURL, UBOOL bItems, ETravelType TravelType ) {}

	// General functions.
	virtual UBOOL SafeExec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	void ExecMacro( const TCHAR* Filename, FOutputDevice& Ar );
	virtual void Cleanse( UBOOL Redraw, const TCHAR* TransReset );
	virtual void FinishAllSnaps( ULevel* Level );
	virtual void RedrawLevel( ULevel* Level );
	virtual void ResetSound();
	virtual void AddActor( ULevel* Level, UClass* Class, FVector V );
	virtual void NoteSelectionChange( ULevel* Level );
	virtual void NoteActorMovement( ULevel* Level );
	virtual void SetPivot( FVector NewPivot, UBOOL SnapPivotToGrid, UBOOL MoveActors );
	virtual void ResetPivot();
	virtual void UpdatePropertiesWindows();
	virtual UTransactor* CreateTrans();

	// Editor mode virtuals from UnEdCam.cpp.
	virtual void edcamSetMode( INT Mode );
	virtual int edcamMode( UViewport* Viewport );

	// Editor CSG virtuals from UnEdCsg.cpp.
	virtual void csgPrepMovingBrush( ABrush* Actor );
	virtual void csgCopyBrush( ABrush* Dest, ABrush* Src, DWORD PolyFlags, DWORD ResFlags, UBOOL IsMovingBrush );
	virtual ABrush*	csgAddOperation( ABrush* Actor, ULevel* Level, DWORD PolyFlags, ECsgOper CSG );
	virtual void csgRebuild( ULevel* Level );
	virtual const TCHAR* csgGetName( ECsgOper CsgOper );

	// Editor EdPoly/BspSurf assocation virtuals from UnEdCsg.cpp.
	virtual INT polyFindMaster( UModel* Model, INT iSurf, FPoly& Poly );
	virtual void polyUpdateMaster( UModel* Model, INT iSurf, INT UpdateTexCoords, INT UpdateBase );

	// Bsp Poly search virtuals from UnEdCsg.cpp.
	virtual void polyFindByFlags( UModel* Model, DWORD SetBits, DWORD ClearBits, POLY_CALLBACK Callback );
	virtual void polyFindByBrush( UModel* Model, ABrush* Actor, INT BrushPoly, POLY_CALLBACK Callback );
	virtual void polySetAndClearPolyFlags( UModel* Model, DWORD SetBits, DWORD ClearBits, INT SelectedOnly, INT UpdateMaster );

	// Selection.
	virtual void SelectNone( ULevel* Level, UBOOL Notify );

	// Bsp Poly selection virtuals from UnEdCsg.cpp.
	virtual void polyResetSelection( UModel* Model );
	virtual void polySelectAll ( UModel* Model );
	virtual void polySelectMatchingGroups( UModel* Model );
	virtual void polySelectMatchingItems( UModel* Model );
	virtual void polySelectCoplanars( UModel* Model );
	virtual void polySelectAdjacents( UModel* Model );
	virtual void polySelectAdjacentWalls( UModel* Model );
	virtual void polySelectAdjacentFloors( UModel* Model );
	virtual void polySelectAdjacentSlants( UModel* Model );
	virtual void polySelectMatchingBrush( UModel* Model );
	virtual void polySelectMatchingTexture( UModel* Model );
	virtual void polySelectReverse( UModel* Model );
	virtual void polyMemorizeSet( UModel* Model );
	virtual void polyRememberSet( UModel* Model );
	virtual void polyXorSet( UModel* Model );
	virtual void polyUnionSet( UModel* Model );
	virtual void polyIntersectSet( UModel* Model );
	virtual void polySelectZone( UModel *Model );

	// Poly texturing virtuals from UnEdCsg.cpp.
	virtual void polyTexPan( UModel* Model, INT PanU, INT PanV, INT Absolute );
	virtual void polyTexScale( UModel* Model,FLOAT UU, FLOAT UV, FLOAT VU, FLOAT VV, UBOOL Absolute );
	virtual void polyTexAlign( UModel* Model, enum ETexAlign TexAlignType, DWORD Texels );

	// Map brush selection virtuals from UnEdCsg.cpp.
	virtual void mapSelectOperation( ULevel* Level, ECsgOper CSGOper );
	virtual void mapSelectFlags(ULevel* Level, DWORD Flags );
	virtual void mapSelectFirst( ULevel* Level );
	virtual void mapSelectLast( ULevel* Level );
	virtual void mapBrushGet( ULevel* Level );
	virtual void mapBrushPut( ULevel* Level );
	virtual void mapSendToFirst( ULevel* Level );
	virtual void mapSendToLast( ULevel* Level );
	virtual void mapSetBrush( ULevel* Level, enum EMapSetBrushFlags PropertiesMask, _WORD BrushColor, FName Group, DWORD SetPolyFlags, DWORD ClearPolyFlags );

	// Editor actor virtuals from UnEdAct.cpp.
	virtual void edactSelectAll( ULevel* Level );
	virtual void edactSelectInside( ULevel* Level );
	virtual void edactSelectInvert( ULevel* Level );
	virtual void edactSelectOfClass( ULevel* Level, UClass* Class );
	virtual void edactDeleteSelected( ULevel* Level );
	virtual void edactDuplicateSelected( ULevel* Level );
	virtual void edactCopySelected( ULevel* Level );
	virtual void edactPasteSelected( ULevel* Level );
	virtual void edactReplaceSelectedBrush( ULevel* Level );
	virtual void edactReplaceSelectedWithClass( ULevel* Level, UClass* Class );
	virtual void edactReplaceClassWithClass( ULevel* Level, UClass* Class, UClass* WithClass );
	virtual void edactAlignVertices( ULevel* Level );
	virtual void edactHideSelected( ULevel* Level );
	virtual void edactHideUnselected( ULevel* Level );
	virtual void edactUnHideAll( ULevel* Level );
	virtual void edactApplyTransform( ULevel* Level );

	// Bsp virtuals from UnBsp.cpp.
	virtual void bspRepartition( UModel* Model, INT iNode, UBOOL Simple );
	virtual INT bspAddVector( UModel* Model, FVector* V, UBOOL Exact );
	virtual INT bspAddPoint( UModel* Model, FVector* V, UBOOL Exact );
	virtual INT bspNodeToFPoly( UModel* Model, INT iNode, FPoly* EdPoly );
	virtual void bspBuild( UModel* Model, enum EBspOptimization Opt, INT Balance, INT RebuildSimplePolys, INT iNode );
	virtual void bspRefresh( UModel* Model, UBOOL NoRemapSurfs );
	virtual void bspCleanup( UModel* Model );
	virtual void bspBuildBounds( UModel* Model );
	virtual void bspBuildFPolys( UModel* Model, UBOOL SurfLinks, INT iNode );
	virtual void bspMergeCoplanars( UModel* Model, UBOOL RemapLinks, UBOOL MergeDisparateTextures );
	virtual INT bspBrushCSG( ABrush* Actor, UModel* Model, DWORD PolyFlags, ECsgOper CSGOper, UBOOL RebuildBounds );
	virtual void bspOptGeom( UModel* Model );
	virtual void bspValidateBrush( UModel* Brush, INT ForceValidate, INT DoStatusUpdate );
	virtual INT	bspAddNode( UModel* Model, INT iParent, enum ENodePlace ENodePlace, DWORD NodeFlags, FPoly* EdPoly );

	// Shadow virtuals (UnShadow.cpp).
	virtual void shadowIlluminateBsp( ULevel* Level, UBOOL Selected );

	// Mesh functions (UnMeshEd.cpp).
	virtual void meshImport( const TCHAR* MeshName, UObject* InParent, const TCHAR* AnivFname, const TCHAR* DataFname, UBOOL Unmirror, UBOOL ZeroTex, INT UnMirrorTex, ULODProcessInfo* LODInfo);
	virtual void meshBuildBounds( UMesh* Mesh );
	virtual void meshLODProcess( ULodMesh* Mesh, ULODProcessInfo* LODInfo);
	virtual void meshDropFrames( UMesh* Mesh, INT StartFrame, INT NumFrame );

	// Visibility.
	virtual void TestVisibility( ULevel* Level, UModel* Model, int A, int B );

	// Scripts.
	virtual int MakeScripts( FFeedbackContext* Warn, UBOOL MakeAll, UBOOL Booting );
	virtual int CheckScripts( FFeedbackContext* Warn, UClass* Class, FOutputDevice& Ar );

	// Topics.
	virtual void Get( const TCHAR* Topic, const TCHAR* Item, FOutputDevice& Ar );
	virtual void Set( const TCHAR* Topic, const TCHAR* Item, const TCHAR* Value );
	virtual void EdCallback( DWORD Code, UBOOL Send );

	// Far-plane Z clipping state control functions.
	virtual void SetZClipping();
	virtual void ResetZClipping();

	// Editor rendering functions.
	virtual void DrawFPoly( struct FSceneNode* Frame, FPoly *Poly, FPlane WireColor, DWORD LineFlags );
	virtual void DrawGridSection( struct FSceneNode* Frame, INT ViewportLocX, INT ViewportSXR, INT ViewportGridY, FVector* A, FVector* B, FLOAT* AX, FLOAT* BX, INT AlphaCase );
	virtual void DrawWireBackground( struct FSceneNode* Frame );
	virtual void DrawLevelBrushes( struct FSceneNode* Frame, UBOOL bStatic, UBOOL bDynamic, UBOOL bActive );
	virtual void DrawLevelBrush( struct FSceneNode* Frame, ABrush* Actor, UBOOL bStatic, UBOOL bDynamic, UBOOL bActive );
	virtual void DrawBoundingBox( struct FSceneNode* Frame, FBox* Bound, AActor* Actor );
};

/*-----------------------------------------------------------------------------
	Parameter parsing functions.
-----------------------------------------------------------------------------*/

EDITOR_API UBOOL GetFVECTOR( const TCHAR* Stream, const TCHAR* Match, FVector& Value );
EDITOR_API UBOOL GetFVECTOR( const TCHAR* Stream, FVector& Value );
EDITOR_API UBOOL GetFROTATOR( const TCHAR* Stream, const TCHAR* Match, FRotator& Rotation, int ScaleFactor );
EDITOR_API UBOOL GetFROTATOR( const TCHAR* Stream, FRotator& Rotation, int ScaleFactor );
EDITOR_API UBOOL GetBEGIN( const TCHAR** Stream, const TCHAR* Match );
EDITOR_API UBOOL GetEND( const TCHAR** Stream, const TCHAR* Match );
EDITOR_API TCHAR* SetFVECTOR( TCHAR* Dest, const FVector* Value );
EDITOR_API UBOOL GetFSCALE( const TCHAR* Stream, FScale& Scale );

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif