/*=============================================================================
	FractalPrivate.h: Fractal texture effects private header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	By Erik de Neve.
=============================================================================*/

/*----------------------------------------------------------------------------
	Dependencies.
----------------------------------------------------------------------------*/

#include "Engine.h"

/*----------------------------------------------------------------------------
	General structures.
----------------------------------------------------------------------------*/

#if !_MSC_VER
	typedef SQWORD __int64;
#endif

// Random numbers.
BYTE SpeedRand();

// Lightning.
struct LineSeg
{
	BYTE Xpos,Ypos,Xlen,Ylen;
};


//
// #Debugging purposes only. 
//
extern "C"
{
	extern DOUBLE LTimeTotal1, LTimeTotal2, LinePixels;
}
__int64  TotalTime0;
__int64  TotalTime1;
__int64  TotalTime2;
__int64  TotalTime3;
__int64  TotalTime4;
__int64  TotalPixels;
  INT    TotalFrames;

INT CycleCount()
{
#if ASM
    INT OutCyc;
    __asm
   {
        xor edx,edx
		xor eax,eax
        //RDTSC;     // Moves Pentium+ time stamp register to EDX:EAX
        _emit 0x0F;
        _emit 0x31;
        //
        mov OutCyc,EAX //only low-order 32-bits needed
    }
    return (OutCyc);
#else
	return 0;
#endif
}

/*----------------------------------------------------------------------------
	UFractalTexture.
----------------------------------------------------------------------------*/

//
// Base fractal texture object.
// warning: Mirrored in FractalTexture.uc.
//

class DLL_EXPORT_CLASS UFractalTexture : public UTexture
{
	DECLARE_ABSTRACT_CLASS(UFractalTexture,UTexture,0)

	// Variables (all transient)
	INT     UMask;
	INT     VMask;
	INT     LightOutput;
    INT     SoundOutput;
    INT     GlobalPhase;
    BYTE    DrawPhase;
    BYTE    AuxPhase;

	// Constructors. Executed on both instantation and reloading of an object.
	UFractalTexture();

	// UObject interface.	
	void PostLoad();
	void PostEditChange() {PostLoad();}

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );

	// UFractalTexture interface.
	virtual void TouchTexture(INT UPos, INT VPos, FLOAT Magnitude) {}
};

/*----------------------------------------------------------------------------
	UFireTexture.
----------------------------------------------------------------------------*/

// Fire constants.
#define MAXSPARKSINIT     1024 /* _Initial_ maximum sparks per fire texture. */
#define MAXSPARKSLIMIT    8192 /* Total allowed ceiling...*/
#define MINSPARKSLIMIT    4    /* Minimum required number */

// Spark types.
enum ESpark
{
	SPARK_Burn				,
	SPARK_Sparkle			,
	SPARK_Pulse				,
	SPARK_Signal			,
	SPARK_Blaze				,
	SPARK_OzHasSpoken		,
	SPARK_Cone				,
	SPARK_BlazeRight		,
	SPARK_BlazeLeft			,
	SPARK_Cylinder			,
	SPARK_Cylinder3D		,
	SPARK_Lissajous 		,
	SPARK_Jugglers   		,
	SPARK_Emit				,
    SPARK_Fountain			,
	SPARK_Flocks			,
	SPARK_Eels				,
	SPARK_Organic			,
	SPARK_WanderOrganic		,
	SPARK_RandomCloud		,
	SPARK_CustomCloud		,
	SPARK_LocalCloud		,
	SPARK_Stars				,
	SPARK_LineLightning		,
	SPARK_RampLightning		,
    SPARK_SphereLightning	,
    SPARK_Wheel				,
	SPARK_Gametes    		,
	SPARK_Sprinkler			,
	SPARK_LASTTYPE			,
};


// Draw mode types
enum DMode
{
	DRAW_Normal    ,
	DRAW_Lathe     ,
	DRAW_Lathe_2   ,
	DRAW_Lathe_3   ,
	DRAW_Lathe_4   ,
};


// Information about a single spark.
// warning: Mirrored in FireTexture.uc.
class DLL_EXPORT_CLASS FSpark
{
	public:
    BYTE    Type;    // Spark type (ESpark).
    BYTE    Heat;    // Spark heat.
    BYTE    X;       // Spark X location (0 - Xdimension-1).
    BYTE    Y;       // Spark Y location (0 - Ydimension-1).

    BYTE    ByteA;   // X-speed.
    BYTE    ByteB;   // Y-speed.
    BYTE    ByteC;   // Age, Emitter freq.
    BYTE    ByteD;   // Exp.Time.

	friend FArchive& operator<<( FArchive& Ar, FSpark& S )
	{
		return  Ar << S.Type << S.Heat << S.X << S.Y << S.ByteA << S.ByteB << S.ByteC << S.ByteD;
	}
};

//
// A fire texture object.
// warning: Mirrored in FireTexture.uc.
//

class DLL_EXPORT_CLASS UFireTexture : public UFractalTexture
{
	DECLARE_CLASS(UFireTexture,UFractalTexture,0)

	// Persistent variables.
    BYTE		SparkType;    
    BYTE        RenderHeat;
	BITFIELD	bRising:1 GCC_PACK(4);

	// FX specifics
	BYTE		FX_Heat GCC_PACK(4);
	BYTE		FX_Size;
	BYTE		FX_AuxSize;
	BYTE		FX_Area;
	BYTE		FX_Frequency;
	BYTE		FX_Phase;
	BYTE		FX_HorizSpeed;
	BYTE		FX_VertSpeed;
	// Edit-time drawing modes
	BYTE        DrawMode;
	INT         SparksLimit;

	// Sparks
	INT         ActiveSparkNum;
	TArray<FSpark> Sparks;

	// Transient variables.
	INT         OldRenderHeat;
    BYTE        RenderTable[1028];
    BYTE        StarStatus;
	BYTE        PenDownX;
	BYTE        PenDownY;

	// Constructors.
	UFireTexture();
	
	// UObject interface.
	void PostLoad();
	void Serialize( FArchive& Ar );

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Clear( DWORD ClearFlags );
	void ConstantTimeTick();
	void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y );
	void Click( DWORD Buttons, FLOAT X, FLOAT Y );

	// UFractalTexture interface.
	void TouchTexture(INT UPos, INT VPos, FLOAT Magnitude);

	// UFireTexture interface. 
	private:
	void AddSpark(INT SparkX, INT SparkY);
	void DrawSparkLine( INT StartX, INT StartY, INT DestX, INT DestY, INT Density );
	void FirePaint( INT MouseX, INT MouseY, DWORD Buttons );
	void CloseSpark( INT SparkX, INT SparkY );
	void DeleteSparks( INT SparkX, INT SparkY, INT AreaWidth );
	void DrawFlashRamp( LineSeg LL, BYTE Color1, BYTE Color2 );
	void MoveSpark( FSpark* Spark );
	void MoveSparkTwo( FSpark* Spark );
	void MoveSparkXY( FSpark* Spark, SBYTE Xspeed, SBYTE Yspeed );
	void MoveSparkAngle( FSpark* Spark, BYTE Direction );
	void RedrawSparks();
	void PostDrawSparks();
	void TempDrawSpark (INT PosX, INT PosY, INT Intensity );

};

/*----------------------------------------------------------------------------
	UWaterTexture.
----------------------------------------------------------------------------*/

// Water constants.
#define MaxDrops 256 /* Maximum number of drops in a water texture. */

// Wave drops.
enum EDrop
{
	DROP_FixedDepth			,  // Fixed depth spot, A=depth
	DROP_PhaseSpot			,  // Phased depth spot, A=frequency B=phase
	DROP_ShallowSpot		,  // Shallower phased depth spot, A=frequency B=phase
	DROP_HalfAmpl           ,  // Half-amplitude (only 128+ values)
	DROP_RandomMover		,  // Randomly moves around
	DROP_FixedRandomSpot	,  // Fixed spot with random output
	DROP_WhirlyThing		,  // Moves in small circles, A=speed B=depth
	DROP_BigWhirly			,  // Moves in large circles, A=speed B=depth
	DROP_HorizontalLine		,  // Horizontal line segment
	DROP_VerticalLine		,  // Vertical line segment
	DROP_DiagonalLine1		,  // Diagonal '/'
	DROP_DiagonalLine2		,  // Diagonal '\'
	DROP_HorizontalOsc		,  // Horizontal oscillating line segment
	DROP_VerticalOsc		,  // Vertical oscillating line segment
	DROP_DiagonalOsc1		,  // Diagonal oscillating '/'
	DROP_DiagonalOsc2		,  // Diagonal oscillating '\'
	DROP_RainDrops			,  // General random raindrops, A=depth B=distribution radius
	DROP_AreaClamp          ,  // Clamp spots to indicate shallow/dry areas
	DROP_LeakyTap			,  // Interval drops
	DROP_DrippyTap			,  // Irregular interval drops
};


// Information about a drop of water in a water texture.
struct DLL_EXPORT_CLASS FDrop
{
    BYTE Type;     // Water drop type (EDrop).
    BYTE Depth;    // Depth.
    BYTE X;        // Spark X location (0 - Xdimension-1).
    BYTE Y;		   // Spark Y location (0 - Ydimension-1).
    BYTE ByteA;    // X-speed.
    BYTE ByteB;    // Y-speed.
	BYTE ByteC;    // Age, Emitter freq.
	BYTE ByteD;    // Exp.Time.
};

//
// A Wave texture object.
// warning: Mirrored in WaterTexture.uc.
//
class DLL_EXPORT_CLASS UWaterTexture : public UFractalTexture
{
	DECLARE_ABSTRACT_CLASS(UWaterTexture,UFractalTexture,0)

	// Persistent variables:
	BYTE		DropType;       // Warning: force all 'enum' types to be BYTEs.	
    BYTE		WaveAmp; 
	// FX specifics
	BYTE		FX_Frequency;
	BYTE		FX_Phase;
	BYTE        FX_Amplitude;
	BYTE		FX_Speed;
	BYTE        FX_Radius;
	BYTE		FX_Size;
	BYTE		FX_Depth;
	BYTE        FX_Time;
	// Drops.
    INT			NumDrops;
    FDrop		Drops[MaxDrops];

    // Non-Persistent variables:
    BYTE*   SourceFields;
    BYTE    RenderTable[1028];
    BYTE	WaveTable[1536];
    BYTE	WaveParity;
	INT     OldWaveAmp;
	
	// Constructors.
	UWaterTexture();

	// UObject interface.
	void PostLoad();
	void Destroy();

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Clear( DWORD ClearFlags );
	void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y );
	void Click( DWORD Buttons, FLOAT X, FLOAT Y );

	// UFractalTexture interface.
	void TouchTexture(INT UPos, INT VPos, FLOAT Magnitude);

	// UWaterTexture interface.
	void CalculateWater();
	void WaterRedrawDrops();
	private:
	void AddDrop( INT DropX, INT DropY );
	void WaterPaint( INT MouseX, INT MouseY, DWORD Buttons );
	void DeleteDrops( INT DropX, INT DropY, INT AreaWidth );
};

/*----------------------------------------------------------------------------
	UWaveTexture.
----------------------------------------------------------------------------*/

class DLL_EXPORT_CLASS UWaveTexture : public UWaterTexture
{
	DECLARE_CLASS(UWaveTexture,UWaterTexture,0)

	// Persistent variables.
	BYTE    BumpMapLight;
	BYTE    BumpMapAngle;
	BYTE    PhongRange;
	BYTE    PhongSize;

	// Constructors.
	UWaveTexture();
    
	// UObject interface.
	void PostLoad();

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Clear( DWORD ClearFlags );
	void ConstantTimeTick();

	// UWaveTexture interface.
	void SetWaveLight();
};

/*----------------------------------------------------------------------------
	UWetTexture.
----------------------------------------------------------------------------*/

class DLL_EXPORT_CLASS UWetTexture : public UWaterTexture
{
	DECLARE_CLASS(UWetTexture,UWaterTexture,0)

    // Persistent variables
    UTexture*	SourceTexture;

	// Texture pointers
	UTexture*	OldSourceTex;  
	BYTE*       LocalSourceBitmap;

	// Constructors.
	UWetTexture();
	
	// UObject interface.
	void PostLoad();
	void Destroy();

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Clear( DWORD ClearFlags );
	void ConstantTimeTick();
	void Lock( FTextureInfo& TextureInfo, DOUBLE Time, INT LOD, URenderDevice* RenDev )
	{
		if( SourceTexture )
		{
			FTextureInfo TempInfo;
			if( SourceTexture != this )
			{
				SourceTexture->Lock( TempInfo, Time, 0, NULL );
				SourceTexture->Unlock( TempInfo );
			}
		}
		Super::Lock( TextureInfo, Time, 0, RenDev );
	}

private:
	// UWetTexture interface.
	void SetRefractionTable();
	void ApplyWetTexture();
};


/*----------------------------------------------------------------------------
	UIceTexture.
----------------------------------------------------------------------------*/

// Ice constants.
#define MaxGuideKeys 64

// Ice movement definitions.
enum PanningType
{
    SLIDE_Linear,
	SLIDE_Circular,
	SLIDE_Gestation,
	SLIDE_WavyX,
	SLIDE_WavyY,
};

// A keypoint for movement.
// warning: Mirrored in IceTexture.uc.
struct DLL_EXPORT_CLASS KeyPoint
{
	BYTE    X;       // Keypoint X - fix
	BYTE    Y;       // Keypoint Y - fix
	BYTE    SpeedX;             
	BYTE    SpeedY;  

	BYTE    Pause1;
	BYTE    Pause2;
	BYTE    Pause3;
	BYTE    Pause4;
};

//
// An Ice texture object.
// warning: Mirrored in IceTexture.uc.
//
class DLL_EXPORT_CLASS UIceTexture : public UFractalTexture
{
	DECLARE_CLASS(UIceTexture,UFractalTexture,0)

	// Persistent variables.
    UTexture*   GlassTexture;
    UTexture*   SourceTexture;
    BYTE        PanningStyle;
	BYTE        TimeMethod;
    BYTE        HorizPanSpeed;
    BYTE        VertPanSpeed;
	BYTE        Frequency;
	BYTE        Amplitude;
	BITFIELD    MoveIce:1;   // Mirrored as BOOLEAN in C++
	FLOAT       MasterCount;
	FLOAT       UDisplace;
	FLOAT 		VDisplace;
	FLOAT       UPosition;
	FLOAT       VPosition;

	// Transient IceTexture Parameters.
	FLOAT       TickAccu;
	INT         OldUDisp;
	INT         OldVDisp;
	UTexture*   OldGlassTex;
	UTexture*	OldSourceTex;
	BYTE*       LocalSourceBitmap;
	INT         ForceRefresh;

	// Constructors.
	UIceTexture();
    
	// UObject interface.
	void PostLoad();
    void Destroy(); 

	// UTexture interface.
	void Init( INT InUSize, INT InVSize );
	void Clear( DWORD ClearFlags );
	void ConstantTimeTick();
	void Tick(FLOAT DeltaSeconds);
	void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y );
	void Click( DWORD Buttons, FLOAT X, FLOAT Y );
	void Lock( FTextureInfo& TextureInfo, DOUBLE Time, INT LOD, URenderDevice* RenDev )
	{
		if( GlassTexture )
		{
			FTextureInfo TempInfo;
			if( GlassTexture != this )
			{
				GlassTexture->Lock( TempInfo, Time, 0, NULL );
				GlassTexture->Unlock( TempInfo );
			}
		}
		if( SourceTexture )
		{
			FTextureInfo TempInfo;
			if( SourceTexture != this )
			{
				SourceTexture->Lock( TempInfo, Time, 0, NULL );
				SourceTexture->Unlock(TempInfo);
			}
		}
		Super::Lock( TextureInfo, Time, 0, RenDev );
	}

private:
	void MoveIcePosition(FLOAT VTicks);
	void RenderIce(FLOAT DeltaTime);
	void BlitTexIce();
	void BlitIceTex();
};

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/