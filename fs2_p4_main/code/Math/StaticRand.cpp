/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "StaticRand.h"
#include "vecmat.h"
#endif

int Semirand_inited = 0;
int Semirand[SEMIRAND_MAX];

//	Initialize Semirand array.
void init_semirand()
{
	int	i;

	Semirand_inited = 1;

	for (i=0; i<SEMIRAND_MAX; i++)
		Semirand[i] = (myrand() << 15) + myrand();
}


//	Return a fairly random 32 bit value given a reasonably small number.
int static_rand(int num)
{
	int	a, b, c;

	if (!Semirand_inited)
		init_semirand();

	a = num & (SEMIRAND_MAX - 1);
	b = (num >> SEMIRAND_MAX_LOG) & (SEMIRAND_MAX - 1);
	c = (num >> (2 * SEMIRAND_MAX_LOG)) & (SEMIRAND_MAX - 1);

	return Semirand[a] ^ Semirand[b] ^ Semirand[c];
}

//	Return a random value in 0.0f .. 1.0f- (ie, it will never return 1.0f).
float static_randf(int num)
{
	int	a;

	a = static_rand(num);

	return (a & 0xffff) / 65536.0f;
}

float static_randf_range(int num, float min, float max)
{
	float	rval;
	
	rval = static_randf(num);
	rval = rval * (max - min) + min;

	return rval;
}


void static_randvec(int num, vector *vp)
{
	vp->x = static_randf(num) - 0.5f;
	vp->y = static_randf(num+1) - 0.5f;
	vp->z = static_randf(num+2) - 0.5f;

	vm_vec_normalize_quick(vp);
}

/////////////////////////////////////////////////////////////////////
// Alternate random number generator, that doesn't affect rand() sequence
/////////////////////////////////////////////////////////////////////
#define RND_MASK	0x6000
#define RND_MAX	0x7fff
int Rnd_seed = 1;

// Seed the random number generator.  Doesn't have to be called.
void srand_alt(int seed)
{
	Rnd_seed = seed;
}

// Get a random integer between 1 and RND_MAX
int rand_alt()
{
	static int x=Rnd_seed;
	int old_x;
	old_x = x;
	x >>= 1;
	if ( old_x & 1 ) {
		x ^= RND_MASK;
	}
	return x;
}

// Get a random float between 0 and 1.0
float frand_alt()
{
	int r = rand_alt();
	return i2fl(r)/RND_MAX;
}