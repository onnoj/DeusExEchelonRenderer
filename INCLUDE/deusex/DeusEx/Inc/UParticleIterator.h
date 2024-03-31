//
// UParticleIterator.h
//
// DEUS_EX CNN
// Converted from script to C++ for 40% speed improvement
//

#include "Engine.h"

#ifndef _UPARTICLEITERATOR_H_
#define _UPARTICLEITERATOR_H_

#define MAX_PARTICLES 64

public:

   // UParticleIterator interface
	AActor* CurrentItem();

private:
	void DeleteParticle(INT);

#endif	// _UPARTICLEITERATOR_H_

// end of UParticleIterator.h
