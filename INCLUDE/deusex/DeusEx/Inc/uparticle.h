//
// UParticle.h
//
// DEUS_EX AMSD
//

#ifndef _UPARTICLE_H_
#define _UPARTICLE_H_

struct FsParticle
{
   BITFIELD	bActive:1 GCC_PACK(4);
   FVector		initVel GCC_PACK(4);
   FVector		Velocity;
   FVector		Location;
   FLOAT		DrawScale;
   FLOAT		ScaleGlow;
   FLOAT		LifeSpan;
};

#endif	// _UPARTICLE_H_

// end of UParticle.h
