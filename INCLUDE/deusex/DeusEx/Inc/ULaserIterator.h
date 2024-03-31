//
// ULaserIterator.h
//
// DEUS_EX CNN
// Converted from script to C++ for 40% speed improvement
//

#include "Engine.h"

#ifndef _ULASERITERATOR_H_
#define _ULASERITERATOR_H_

#define MAX_BEAMS 8

/*-----------------------------------------------------------------------------
	ULaserIterator.
-----------------------------------------------------------------------------*/

class DEUSEX_API ULaserIterator : public URenderIterator
{
	DECLARE_CLASS(ULaserIterator,URenderIterator,0)

public:
	struct sBeam
	{
		BITFIELD	bActive:1 GCC_PACK(4);
		FVector		Location GCC_PACK(4);
		FRotator	Rotation;
		FLOAT		Length;
		INT			numSegments;
	};

	sBeam Beams[MAX_BEAMS];
	FVector prevLoc, prevRand, savedLoc;
	FRotator savedRot;
	INT nextItem;
	AActor* proxy;
	BITFIELD bRandomBeam:1;

	// ULaserIterator interface
	AActor* CurrentItem();
};

#endif	// _ULASERITERATOR_H_

// end of ULaserIterator.h
