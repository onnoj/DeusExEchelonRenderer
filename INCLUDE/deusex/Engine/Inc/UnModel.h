/*=============================================================================
	UnModel.h: Unreal UModel definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UModel.
-----------------------------------------------------------------------------*/

//
// Model objects are used for brushes and for the level itself.
//
enum {MAX_NODES  = 65536};
enum {MAX_POINTS = 128000};
class ENGINE_API UModel : public UPrimitive
{
#ifndef NODECALS
	DECLARE_CLASS(UModel,UPrimitive,0)
#else
	DECLARE_CLASS(UModel,UPrimitive,CLASS_RuntimeStatic)
#endif /*NODECALS*/

	// Arrays and subobjects.
	UPolys*					Polys;
	TTransArray<FBspNode>	Nodes;
	TTransArray<FVert>      Verts;
	TTransArray<FVector>	Vectors;
	TTransArray<FVector>	Points;
	TTransArray<FBspSurf>	Surfs;
	TArray<FLightMapIndex>	LightMap;
	TArray<BYTE>			LightBits;
	TArray<FBox>			Bounds;
	TArray<INT>				LeafHulls;
	TArray<FLeaf>			Leaves;
	TArray<AActor*>			Lights;

	// Other variables.
	UBOOL					RootOutside;
	UBOOL					Linked;
	INT						MoverLink;
	INT						NumSharedSides;
	INT						NumZones;
	FZoneProperties			Zones[FBspNode::MAX_ZONES];

	// Constructors.
	UModel()
	: RootOutside( 1 )
	, Surfs( this )
	, Vectors( this )
	, Points( this )
	, Verts( this )
	, Nodes( this )
	{
		EmptyModel( 1, 0 );
	}
	UModel( ABrush* Owner, UBOOL InRootOutside=1 );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void PostLoad();

	// UPrimitive interface.
	UBOOL PointCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			Location,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	UBOOL LineCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		FVector			End,
		FVector			Start,
		FVector			Extent,
		DWORD           ExtraNodeFlags
	);
	FBox GetCollisionBoundingBox( const AActor *Owner ) const;
	FBox GetRenderBoundingBox( const AActor* Owner, UBOOL Exact );

	// UModel interface.
	void Modify( UBOOL DoTransArrays=0 );
	void BuildBound();
	void Transform( ABrush* Owner );
	void EmptyModel( INT EmptySurfInfo, INT EmptyPolys );
	void ShrinkModel();
	UBOOL PotentiallyVisible( INT iLeaf1, INT iLeaf2 );
	BYTE FastLineCheck( FVector End, FVector Start );

	// UModel transactions.
	void ModifySelectedSurfs( UBOOL UpdateMaster );
	void ModifyAllSurfs( UBOOL UpdateMaster );
	void ModifySurf( INT Index, UBOOL UpdateMaster );

	// UModel collision functions.
	typedef void (*PLANE_FILTER_CALLBACK )(UModel *Model, INT iNode, int Param);
	typedef void (*SPHERE_FILTER_CALLBACK)(UModel *Model, INT iNode, int IsBack, int Outside, int Param);
	FPointRegion PointRegion( AZoneInfo* Zone, FVector Location ) const;
	FLOAT FindNearestVertex
	(
		const FVector	&SourcePoint,
		FVector			&DestPoint,
		FLOAT			MinRadius,
		INT				&pVertex
	) const;
	void PrecomputeSphereFilter
	(
		const FPlane	&Sphere
	);
	FLightMapIndex* GetLightMapIndex( INT iSurf )
	{
		guard(UModel::GetLightMapIndex);
		if( iSurf == INDEX_NONE ) return NULL;
		FBspSurf& Surf = Surfs(iSurf);
		if( Surf.iLightMap==INDEX_NONE || !LightMap.Num() ) return NULL;
		return &LightMap(Surf.iLightMap);
		unguard;
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
