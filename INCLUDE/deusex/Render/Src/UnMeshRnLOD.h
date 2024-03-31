/*=============================================================================
	UnMeshRnLOD.cpp: Unreal mesh LOD rendering.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Erik de Neve

	Note:
		* Included by UnMeshRn.cpp.
=============================================================================*/

//
// Structure used by DrawLodMesh for sorting triangles.
//
struct FMeshFaceSort
{
	FMeshFace* Face;
	INT Key;
};
QSORT_RETURN CDECL CompareFaceKey( const FMeshFaceSort* A, const FMeshFaceSort* B )
{
	return B->Key - A->Key;
}

//
// Draw a mesh map with level-of-detail support.
//
void URender::DrawLodMesh
(
	FSceneNode*		Frame,
	AActor*			Owner,
	AActor*			LightSink,
	FSpanBuffer*	SpanBuffer,
	AZoneInfo*		Zone,
	const FCoords&	Coords,
	FVolActorLink*	LeafLights,
	FActorLink*		Volumetrics,
	DWORD			ExtraFlags
)
{
	guard(URender::DrawLodMesh);

	STAT(clock(GStat.MeshTime));
	FMemMark Mark(GMem);

	ExtraFlags |= PF_Flat; /* LOD doesn't support curved surfaces (yet) */

	ULodMesh*  Mesh = (ULodMesh*)Owner->Mesh;
	FVector Hack = FVector(0,-8,0);
	UBOOL SoftwareRendering =  Frame->Viewport->RenDev->SpanBased;
	UBOOL NotWeaponHeuristic= (Owner->Owner!=Frame->Viewport->Actor);

	//
	// ShapeLODMode: LOD drawing mode.
	//
	// 0    Draw everything at full (or at ShapeLODFix) detail.
	// 1    Normal level-of-detail mode
	// 2    Disable smooth morphing.
	// 3    Ignore the field-of-view (great for debugging)
	// 4    Disable smooth morphing, ignore field-of-view.
	// 

	// Skip LOD strength calculation if at all possible.
	FLOAT TargetSubset = Mesh->ModelVerts;
	INT   VertexSubset = Mesh->ModelVerts;
	INT   NonMorphSubset = 0;  //avoid debug compiler warning

	UBOOL DoMorph = false;
	UBOOL DoLOD =( ( *(DWORD*)&Mesh->LODStrength != 0 ) && (Mesh->CollapsePointThus.Num() != 0) );

	if( DoLOD )
	{
		DoMorph = ( (ShapeLODMode & 1) && (Mesh->LODMorph > 0.0f) );
		FLOAT FovBias  = appTan( Frame->Viewport->Actor->FovAngle*(PI/360.f) );
		// Some debugging modes disable FOV bias:
		if ( ShapeLODMode >= 3 ) 
		{
			FovBias = 1.0f; 
		}

		// Complexity bias: effectively, stronger LOD for more complex meshes.
		FLOAT CpxBias =  0.25f + 0.75f*Mesh->ModelVerts/250.0f; // about even for 250-vertex meshes.
		FLOAT ResolutionBias = 0.3 + 0.7*Frame->FX/640.f; // Moderated resolution influence.
		// Z coordinate :in units. 60 units is about the player's height. 
		FLOAT Z = Owner->Location.TransformPointBy(Coords).Z - Mesh->LODZDisplace;
		FLOAT DetailDiv = Mesh->LODStrength * GlobalShapeLOD * GlobalShapeLODAdjust * FovBias * Max(1.0f,Z) * CpxBias;  	
		FLOAT MeshVertLOD  = 430.f * ResolutionBias * Owner->DrawScale * Owner->LODBias * Mesh->MeshScaleMax / DetailDiv;  
		// Overscaling > 1.0 allowed; actually needed for the LOD morphing. 
		//  150.0 was the initial setting.   340.f seems useful.
		//  430.0 is Steve's 224 conservative setting.

		// Command line debugging variables.
		if ( ShapeLODMode == 0 )
		{
			MeshVertLOD = 1.0f;
			if( ShapeLODFix != 0.f)
			{
				MeshVertLOD = ShapeLODFix;
			}
		}
		TargetSubset    = Max( (FLOAT)Mesh->LODMinVerts, (FLOAT)Mesh->ModelVerts * MeshVertLOD  );
		VertexSubset    = Min( appRound(TargetSubset), Mesh->ModelVerts );
	}


	// Get transformed verts.
	UBOOL bWire = Frame->Viewport->IsOrtho() || Frame->Viewport->Actor->RendMap==REN_Wire;

	// Allocate the necessary amount of our vertex lighting/transforming/texturing structures.
	FTransTexture* AllSamples;	
	AllSamples = New<FTransTexture>(GMem, VertexSubset + Mesh->SpecialVerts); 
	// The real samples start after the special-coordinate ones.
	FTransTexture* Samples = &AllSamples[Mesh->SpecialVerts]; 

	guardSlow(Transform);
	STAT(clock(GStat.MeshGetFrameTime));
	// On top of a possibly changed VertexSubSet, we always get Mesh->SpecialVerts extra vertices returned.
	Mesh->GetFrame( &AllSamples->Point, sizeof(AllSamples[0]), bWire ? GMath.UnitCoords : Coords, Owner, VertexSubset );
	STAT(unclock(GStat.MeshGetFrameTime));
	unguardSlow;
	

	// Smooth morphing.
	if ( DoMorph ) 
	{
		guardSlow(MorphPrepare);
		NonMorphSubset = appRound( TargetSubset * (1.0f - Mesh->LODMorph) ); 

		if( NonMorphSubset >= VertexSubset )
		{
			DoMorph = false;			
		}
		else
		{
			FLOAT InvLODMorph = 1.0f/( Mesh->LODMorph * TargetSubset );
			// prepare an upper subset for morphing.
			for ( INT i=VertexSubset-1; i>NonMorphSubset; i-- ) 
			{
				// Vertices morph according to their closeness to collapse.
				FLOAT Alpha = Min( 1.0f, (FLOAT)(i-NonMorphSubset) * InvLODMorph );
				// Only go down once. 
				INT Morph2 = Mesh->CollapsePointThus(i);
				if ( Morph2 > 0 )
				{
					// because we count DOWN, morphed samples don't hinder each other.
					Samples[i].Point += ( Samples[Morph2].Point - Samples[i].Point ) * Alpha;			
				}
				else
				{
					Alpha = 0.0f;
				}			
				// Stored in U before we need it later as texture coordinate.
				Samples[i].U = Alpha;
			}
		}
		unguardSlow;
	}

	// LOD codepath not necessary in several cases:
	if( !DoMorph && ( VertexSubset >= Mesh->ModelVerts ) ) 
	{
		DoLOD = false; 
	}

	// Compute outcodes.
	// If resulting Outcode & FVF_OutReject == 0  then entire mesh is out of view.
	DWORD MeshOutcode = FVF_OutReject;
	DWORD WeaponOutcode = FVF_OutReject;
	guardSlow(Outcode);
	for( INT i=0; i<Mesh->SpecialVerts; i++ )
	{
		AllSamples[i].Normal = FPlane(0,0,0,0);
		AllSamples[i].ComputeOutcode( Frame );
		WeaponOutcode &= AllSamples[i].Flags;
	}
	for( i=0; i<VertexSubset; i++ )
	{
		Samples[i].Light.X = 0;  // Indicate unprocessed vertex.
		Samples[i].Normal = FPlane(0,0,0,0);
		Samples[i].ComputeOutcode( Frame );
		MeshOutcode &= Samples[i].Flags;
	}
	unguardSlow;

	// Special coordinates setup.
	HasSpecialCoords = 0;
	if ( WeaponOutcode == 0 ) // ( Mesh->SpecialFaces.Num() )
	{
		// Only the first SpecialFace is used - for now.
		FMeshFace& Face = Mesh->SpecialFaces(0);  
		FTransform& V0  = AllSamples[Face.iWedge[0]];
		FTransform& V1  = AllSamples[Face.iWedge[1]];
		FTransform& V2  = AllSamples[Face.iWedge[2]];

		// See if potentially visible. Warning: potential flickering - the weapon itself 
		// might be visible depending on its size, even while these 3 vertices aren't ?
		if ( !(V0.Flags & V1.Flags & V2.Flags) ) 
		{
			FCoords C;
			C.Origin      = FVector(0,0,0);
			C.XAxis	      = (V1.Point - V0.Point).SafeNormal();
			C.YAxis	      = (C.XAxis ^ (V0.Point - V2.Point)).SafeNormal();
			C.ZAxis	      = C.YAxis ^ C.XAxis;
			FVector Mid   = 0.5*(V0.Point + V2.Point);
			SpecialCoords = GMath.UnitCoords * Mid * C;
			HasSpecialCoords = 1;
		}
	}

	// Render a wireframe view.
	if ( bWire )
	{
		guardSlow(Wireframe);
		// Render each wireframe triangle.
		FPlane Color = Owner->bSelected ? FPlane(.2,.8,.1,0) : FPlane(.6,.4,.1,0);

		for( INT i=0; i<Mesh->Faces.Num(); i++ )
		{			
			// Draw only if face's FaceLevel indicates it falls within our vertex budget.
			if( Mesh->FaceLevel(i) <= VertexSubset ) 
			{			
				FMeshFace& Face = Mesh->Faces(i);  
				INT LVert[3]; 
				for( INT v=0; v<3; v++ )
				{
					INT WedgeIndex = Face.iWedge[v];
					INT LODVertIndex =  Mesh->Wedges(WedgeIndex).iVertex;

					// Go down LOD wedge collapse list until below the current LOD vertex count.				
					while( LODVertIndex >= VertexSubset ) // + Mesh->SpecialVerts ??
					{
						WedgeIndex = Mesh->CollapseWedgeThus(WedgeIndex);
						LODVertIndex = Mesh->Wedges(WedgeIndex).iVertex;
					}					
					LVert[v] = LODVertIndex;
				}

				//Render.
				FVector*  P1 = &Samples[ LVert[2] ].Point;
				for( int j=0; j<3; j++ )
				{
					FVector* P2 = &Samples[ LVert[j] ].Point;					
					Frame->Viewport->RenDev->Draw3DLine( Frame, Color, LINE_DepthCued, *P1, *P2 );					
					P1 = P2;
				}
			}
		}
		// Render any special/weapon triangles: for debugging purposes only.
		for( i=0; i<Mesh->SpecialFaces.Num(); i++ )
		{			
			// Draw only if FaceLevel indicates this face falls within our vertex budget.
			FMeshFace& Face = Mesh->SpecialFaces(i);  
			INT LVert[3]; 
			LVert[0] = Face.iWedge[0];
			LVert[1] = Face.iWedge[1];
			LVert[2] = Face.iWedge[2];
			// Render - weapon triangle for debugging..
			FVector*  P1 = &AllSamples[ LVert[2] ].Point;
			for( int j=0; j<3; j++ )
			{
				FVector* P2 = &AllSamples[ LVert[j] ].Point;					
				Frame->Viewport->RenDev->Draw3DLine( Frame, Color, LINE_DepthCued, *P1, *P2 );					
				P1 = P2;
			}
		}
		STAT(unclock(GStat.MeshTime));
		Mark.Pop();
		unguardSlow;
		return;
	}

	// Coloring.
	FLOAT Unlit = Clamp( Owner->ScaleGlow*0.5f + Owner->AmbientGlow/256.f, 0.f, 1.f );
	GUnlitColor = FVector( Unlit, Unlit, Unlit );
	if( GIsEditor && (ExtraFlags & PF_Selected) )
	GUnlitColor = GUnlitColor*0.4 + FVector(0.0,0.6,0.0);

	// Mesh based particle effects with LOD.
	if( Owner->bParticles )
	{
		guardSlow(Particles);
		check(Owner->Texture);
		UTexture* Tex = Owner->Texture->Get( Frame->Viewport->CurrentTime );
		FTransform** SortedPts = New<FTransform*>(GMem,VertexSubset);
		INT Count=0;
		FPlane Color = GUnlitColor;
		if( Owner->ScaleGlow!=1.0 )
		{
			Color *= Owner->ScaleGlow;
			if( Color.X>1.0 ) Color.X=1.0;
			if( Color.Y>1.0 ) Color.Y=1.0;
			if( Color.Z>1.0 ) Color.Z=1.0;
		}
		for( INT i=0; i<VertexSubset; i++ )
		{
			if( !Samples[i].Flags && Samples[i].Point.Z>1.0 )
			{
				Samples[i].Project( Frame );
				SortedPts[Count++] = &Samples[i];
			}
		}
		if( SoftwareRendering )
		{
			Sort( SortedPts, Count );
		}
		for( i=0; i<Count; i++ )
		{
			if( !SortedPts[i]->Flags )
			{
				UTexture* SavedNext = NULL;
				UTexture* SavedCur = NULL;
				if ( Owner->bRandomFrame )
				{	
					// pick texture from multiskins and animate
					Tex = Owner->MultiSkins[appCeil((SortedPts[i]-Samples)/3.f)%8];
					if ( Tex )
					{
						INT Count=1;
						for( UTexture* Test=Tex->AnimNext; Test && Test!=Tex; Test=Test->AnimNext )
							Count++;
						INT Num = Clamp( appFloor(Owner->LifeFraction()*Count), 0, Count-1 );
						while( Num-- > 0 )
							Tex = Tex->AnimNext;
						SavedNext         = Tex->AnimNext;//sort of a hack!!
						SavedCur          = Tex->AnimCur;
						Tex->AnimNext = NULL;
						Tex->AnimCur  = NULL;
					}
				}
				if ( Tex )
				{
					FLOAT XSize = SortedPts[i]->RZ * Tex->USize * Owner->DrawScale;
					FLOAT YSize = SortedPts[i]->RZ * Tex->VSize * Owner->DrawScale;

					Frame->Viewport->Canvas->DrawIcon
					(
						Tex,
						SortedPts[i]->ScreenX - XSize/2,
						SortedPts[i]->ScreenY - XSize/2,
						XSize,
						YSize,
						SpanBuffer,
						Samples[i].Point.Z,
						Color,
						FPlane(0,0,0,0),
						ExtraFlags | PF_TwoSided | Tex->PolyFlags
					);
					Tex->AnimNext = SavedNext;
					Tex->AnimCur  = SavedCur;
				}
			}
		}
		Mark.Pop();
		STAT(unclock(GStat.MeshTime));
		unguardSlow;
		return;
	}


	// Dynamic Face array setup. All faces with valid LOD level get their 3 wedges LOD-processed/morphed,
	// and these get flagged as processed using the (full sized) WedgePool table.	
	TArray<FMeshFaceSort> FacePool;
	TArray<FMeshWedge>    WedgePool;
	WedgePool.AddZeroed( Mesh->Wedges.Num() );
	// Minor kludge: *if* UV==0 and ivertex==0 it will assume an uninitialized one.

	INT MatIndex = -1;
	DWORD MatFlags = 0;

	if (MeshOutcode == 0)
	{
		guardSlow(LODFacesProcessing);

		if( DoLOD )
		{
			for( INT i=0; i<Mesh->Faces.Num(); i++) 
			{
				// This face does not even exist if its FaceLevel 
				// indicates it does not fall within our vertex budget.
				// Optimization: checks & collapses can be skipped for full-detail rendering.
				if( Mesh->FaceLevel(i) <= VertexSubset ) 
				{	
					FMeshFace& Face = Mesh->Faces(i);
					// Faces sorted by materials so don't often change.
					if( MatIndex != Face.MaterialIndex )
					{
						MatIndex = Face.MaterialIndex;
						MatFlags = ExtraFlags | Mesh->Materials(Face.MaterialIndex).PolyFlags;
					}

					FTransTexture* V[3];

					// When morphing: Samples[i].U = alpha.
					
					for( INT w=0; w<3; w++)
					{
						INT iStartWedge = Face.iWedge[w];
						// Only one DWORD.
						FMeshWedge Wedge = Mesh->Wedges(iStartWedge); 

						// Uninitialized wedge ?
						if( *(DWORD*)&WedgePool(iStartWedge) == 0) 
						{
							INT iWedge = iStartWedge;

							while( Wedge.iVertex >= VertexSubset )
							{							
								iWedge = Mesh->CollapseWedgeThus( iWedge );
								Wedge  = Mesh->Wedges(iWedge);						
							};
							
							// Morphing: a fractional collapse.
							if ( DoMorph  && ( Wedge.iVertex > NonMorphSubset ))
							{
								FLOAT Alpha = Samples[Wedge.iVertex].U;
								if( *(DWORD*)&Alpha != 0 ) 
								{
									INT iNext = Mesh->CollapseWedgeThus( iWedge );
									// Actually a different wedge ?
									if (iWedge != iNext)
									{
										FMeshWedge Wedge2 = Mesh->Wedges(iNext);
										// Actually a different UV ?
										if( Wedge.TexUV.U!=Wedge2.TexUV.U
										||	Wedge.TexUV.V!=Wedge2.TexUV.V )
										{
											Wedge.TexUV.U = appRound( (FLOAT)Wedge.TexUV.U + (FLOAT)((FLOAT)Wedge2.TexUV.U - (FLOAT)Wedge.TexUV.U) *  Alpha );
											Wedge.TexUV.V = appRound( (FLOAT)Wedge.TexUV.V + (FLOAT)((FLOAT)Wedge2.TexUV.V - (FLOAT)Wedge.TexUV.V) *  Alpha );
										}
									}
								}
							}
							WedgePool(iStartWedge) = Wedge; // Cache it, including the possibly morphed UV.
						}
						else
						{
							Wedge = WedgePool(iStartWedge);
						}
						V[w] = &Samples[Wedge.iVertex];

					}				
					
					
					// Compute triangle normal whether visible or not.
					FVector FaceNormal = (V[0]->Point-V[1]->Point) ^ (V[2]->Point-V[0]->Point);
					FaceNormal *= DivSqrtApprox(FaceNormal.SizeSquared()+0.001f);

					// Accumulate into normals of all vertices that make up this face.
					V[0]->Normal += FaceNormal;
					V[1]->Normal += FaceNormal;
					V[2]->Normal += FaceNormal;

					// See if potentially visible.
					if( !(V[0]->Flags & V[1]->Flags & V[2]->Flags) )
					{
						if(	(MatFlags & PF_TwoSided) || Frame->Mirror * (V[0]->Point| FaceNormal ) < 0.0 )
						{					
							// Indicate these vertices need to be lit later.
							V[0]->Light.X = -1;
							V[1]->Light.X = -1;
							V[2]->Light.X = -1;

							// This face is visible. Add to the list.
							INT FaceTop = FacePool.Num();
							FacePool.Add();
							FacePool(FaceTop).Face = &Face;

							//Set the sort key ONLY if we're in software.
							if (SoftwareRendering)
							{
								FacePool(FaceTop).Key
								=	NotWeaponHeuristic
								?	appRound( V[0]->Point.Z + V[1]->Point.Z + V[2]->Point.Z )
								:	appRound( FDistSquared(V[0]->Point,Hack)*FDistSquared(V[1]->Point,Hack)*FDistSquared(V[2]->Point,Hack) );
							}
						}
					}
				}
			}
		}
		else // No-LOD variant: no collapses needed.
		{
			for( INT i=0; i<Mesh->Faces.Num(); i++) 
			{
				FMeshFace& Face = Mesh->Faces(i);
				// Faces sorted by materials so don't often change.
				if( MatIndex != Face.MaterialIndex )
				{
					MatIndex = Face.MaterialIndex;
					MatFlags = ExtraFlags | Mesh->Materials(Face.MaterialIndex).PolyFlags;
				}

				FTransTexture* V[3];
				INT iStartWedge0 = Face.iWedge[0];
				INT iStartWedge1 = Face.iWedge[1];
				INT iStartWedge2 = Face.iWedge[2];
				FMeshWedge Wedge0 = Mesh->Wedges(iStartWedge0); 
				FMeshWedge Wedge1 = Mesh->Wedges(iStartWedge1); 
				FMeshWedge Wedge2 = Mesh->Wedges(iStartWedge2); 
				WedgePool(iStartWedge0) = Wedge0;
				WedgePool(iStartWedge1) = Wedge1; 
				WedgePool(iStartWedge2) = Wedge2;
				V[0] = &Samples[Wedge0.iVertex]; 
				V[1] = &Samples[Wedge1.iVertex]; 
				V[2] = &Samples[Wedge2.iVertex];  
				
				// Compute triangle normal whether visible or not.
				FVector FaceNormal = (V[0]->Point-V[1]->Point) ^ (V[2]->Point-V[0]->Point);
				FaceNormal *= DivSqrtApprox(FaceNormal.SizeSquared()+0.001f);

				// Accumulate into normals of all vertices that make up this face.
				V[0]->Normal += FaceNormal;
				V[1]->Normal += FaceNormal;
				V[2]->Normal += FaceNormal;
				
				// See if potentially visible.
				if( !(V[0]->Flags & V[1]->Flags & V[2]->Flags) )
				{
					if(	(MatFlags & PF_TwoSided) || Frame->Mirror * (V[0]->Point| FaceNormal ) < 0.0f )
					{				
						// Indicate these vertices need to be lit later.
						V[0]->Light.X = -1;
						V[1]->Light.X = -1;
						V[2]->Light.X = -1; 

						// This face is visible. Add to the list.
						INT FaceTop = FacePool.Num();
						FacePool.Add();
						FacePool(FaceTop).Face = &Face;

						//Set the sort key ONLY if we're in software.
						if (SoftwareRendering)
						{
							FacePool(FaceTop).Key
							=	NotWeaponHeuristic
							?	appRound( V[0]->Point.Z + V[1]->Point.Z + V[2]->Point.Z )
							:	appRound( FDistSquared(V[0]->Point,Hack)*FDistSquared(V[1]->Point,Hack)*FDistSquared(V[2]->Point,Hack) );
						}
					}
				}
			}
		}
		unguardSlow;
	}

	//
	// Render triangles.
	//

	if( FacePool.Num() )
	{
		guardSlow(Render);

		// DEUS_EX CNN - setup for Matrix easter egg
		UBOOL bMatrixMode = 0;
		UTexture* matrixTex = NULL;
		if (Frame->Viewport->Actor && Frame->Viewport->Actor->IsA(APlayerPawn::StaticClass()) && Frame->Viewport->Actor->Sprite)
		{
			bMatrixMode = 1;
			matrixTex = Frame->Viewport->Actor->Sprite;
		}

		// Fatness.
		UBOOL Fatten   = Owner->Fatness!=128;
		FLOAT Fatness  = (Owner->Fatness/16.0)-8.0;
		//FLOAT Detail   = GlobalMeshLOD * Owner->DrawScale / (0.5f * Frame->RProj.Z * Max(1.0f,Owner->Location.TransformPointBy(Coords).Z));

		// Sort by depth.
		if( SoftwareRendering ) 
		{
			appQsort( &FacePool(0), FacePool.Num(), sizeof(FacePool(0)), (QSORT_COMPARE)CompareFaceKey );
		}

		// Lock the textures.
		UTexture* EnvironmentMap = NULL;
		check(Mesh->Textures.Num()<=ARRAY_COUNT(TextureInfo));
		for( INT i=0; i<Mesh->Textures.Num(); i++ )
		{
			Textures[i] = Mesh->GetTexture( i, Owner );

			// DEUS_EX CNN - Matrix easter egg
			if (bMatrixMode)
				Textures[i] = matrixTex;

			if( Textures[i] )
			{
				Textures[i] = Textures[i]->Get( Frame->Viewport->CurrentTime );
				INT ThisLOD = -1;//Mesh->TextureLOD.Num() ? Clamp<INT>( appCeilLogTwo(1+appFloor(256.f/(Detail*Mesh->TextureLOD(i)*Textures[i]->USize))), 0, 3 ) : 0;
				Textures[i]->Lock( TextureInfo[i], Frame->Viewport->CurrentTime, ThisLOD, Frame->Viewport->RenDev );
				EnvironmentMap = Textures[i];								
			}
		}
		if( Owner->Texture )
			EnvironmentMap = Owner->Texture;
		else if( Owner->Region.Zone && Owner->Region.Zone->EnvironmentMap )
			EnvironmentMap = Owner->Region.Zone->EnvironmentMap;
		else if( Owner->Level->EnvironmentMap )
			EnvironmentMap = Owner->Level->EnvironmentMap;
		if( EnvironmentMap==NULL )
			return;

		// DEUS_EX CNN - Matrix easter egg
		if (bMatrixMode)
			EnvironmentMap = Frame->Viewport->Actor->Sprite;

		check(EnvironmentMap);
		EnvironmentMap->Lock( EnvironmentInfo, Frame->Viewport->CurrentTime, -1, Frame->Viewport->RenDev );

		// Build list of all incident lights on the mesh.
		STAT(clock(GStat.MeshLightSetupTime));
		ExtraFlags |= GLightManager->SetupForActor( Frame, LightSink, LeafLights, Volumetrics );
		STAT(unclock(GStat.MeshLightSetupTime));

		STAT(clock(GStat.MeshLightTime)); 
		// Perform all vertex lighting.

		for( i=0; i<VertexSubset; i++ )
		{
			FTransSample& Vert = Samples[i];
			if( Vert.Light.X == -1 ) // Only light/project if part of a visible triangle.
			{
				// Efficiency warning: FPlane ctor has an implicit dot prodoct.
				Vert.Normal = FPlane( Vert.Point, Vert.Normal * DivSqrtApprox(Vert.Normal.SizeSquared()+0.001f) );					
					
				// Fatten it if desired.
				
				if( Fatten )
				{
					Vert.Point += Vert.Normal * Fatness;
					Vert.ComputeOutcode( Frame );
				}
				

				// Compute effect of each lightsource on this vertex.
				Vert.Light = GLightManager->Light( Vert, ExtraFlags );
				Vert.Fog   = GLightManager->Fog  ( Vert, ExtraFlags );

				// Project it: 
				if( !Vert.Flags ) // Project if visible only.
				{
					Vert.Project( Frame );
				}
			}
		}
		STAT(unclock(GStat.MeshLightTime));

		// Draw the triangles.
		STAT(GStat.MeshPolyCount+=FacePool.Num());

		// Reset cached material indicator.
		MatIndex = -1;
		FTextureInfo* Info = NULL; 

		for( i=0; i<FacePool.Num(); i++ )
		{
			// Set up the triangle.
			FMeshFace &Face = *FacePool(i).Face;

			// Update material if changed since last face.
			if ( MatIndex != Face.MaterialIndex )
			{
				MatIndex = Face.MaterialIndex;
				MatFlags = ExtraFlags | Mesh->Materials( MatIndex ).PolyFlags;
				INT TexIndex =          Mesh->Materials( MatIndex ).TextureIndex;
				Info = ( Textures[TexIndex] && !(MatFlags & PF_Environment)) ? &TextureInfo[TexIndex] : &EnvironmentInfo;
				UScale = Info->UScale * Info->USize/256.0f;
				VScale = Info->VScale * Info->VSize/256.0f;

				// DEUS_EX CNN - force masked textures to render masked polys
				MatFlags |= ((Info->Texture->PolyFlags)&PF_Masked);
			}
			
			// Set up texture coords.
			FTransTexture* Pts[6];
			// Vertex 0,1,2 unrolled assignment.
			FMeshWedge Wedge0 = WedgePool( Face.iWedge[0] );
			FMeshWedge Wedge1 = WedgePool( Face.iWedge[1] );
			FMeshWedge Wedge2 = WedgePool( Face.iWedge[2] );
			Pts[0]    = &Samples[ Wedge0.iVertex ];
			Pts[1]    = &Samples[ Wedge1.iVertex ];
			Pts[2]    = &Samples[ Wedge2.iVertex ];
			Pts[0]->U = Wedge0.TexUV.U * UScale;
			Pts[1]->U = Wedge1.TexUV.U * UScale;
			Pts[2]->U = Wedge2.TexUV.U * UScale;
			Pts[0]->V = Wedge0.TexUV.V * VScale;
			Pts[1]->V = Wedge1.TexUV.V * VScale;
			Pts[2]->V = Wedge2.TexUV.V * VScale;

			if( Frame->Mirror == -1 ) 
					Exchange( Pts[2], Pts[0] );
			RenderSubsurface( Frame, *Info, SpanBuffer, Pts, MatFlags, 0 );
		}

		GLightManager->FinishActor();

		for( i=0; i<Mesh->Textures.Num(); i++ )
		{
			if( Textures[i] ) Textures[i]->Unlock( TextureInfo[i] );
		}
		EnvironmentMap->Unlock( EnvironmentInfo );		

		unguardSlow;
	}


	STAT(GStat.MeshCount++);
	STAT(unclock(GStat.MeshTime));
	Mark.Pop();

	unguardf(( TEXT("(%s)"), Owner->Mesh->GetFullName() ));
}

/*------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------*/