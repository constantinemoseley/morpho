/** @file random.c
 *  @author T J Atherton and others (see below)
 *
 *  @brief Random number generation
*/

#include "random.h"

/* **********************************************************************
 * Splitmix64 (used for initialization purposes only)
 * ********************************************************************** */

/*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)
    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.
    See <http://creativecommons.org/publicdomain/zero/1.0/>.

    This is a fixed-increment version of Java 8's SplittableRandom generator
    See http://dx.doi.org/10.1145/2714064.2660195 and
    http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
    It is a very fast generator passing BigCrush, and it can be useful if
    for some reason you absolutely want 64 bits of state; otherwise, we
    rather suggest to use a xoroshiro128+ (for moderately parallel
    computations) or xorshift1024* (for massively parallel computations)
    generator. */

uint64_t splitmix64_state; /* The state can be seeded with any value. */

/** Get the next random number generated by splitmix64 */
static inline uint64_t splitmix64_next(void) {
    uint64_t z = (splitmix64_state += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

/** Set the seed for splitmix64 */
void splitmix64_seed(uint64_t seed) {
    splitmix64_state=seed;
}

/* **********************************************************************
 * xoshiro256++
 * ********************************************************************** */

/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.

    See <http://creativecommons.org/publicdomain/zero/1.0/>.

    This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
    It has excellent (sub-ns) speed, a state (256 bits) that is large
    enough for any parallel application, and it passes all tests we are
    aware of.

    For generating just floating-point numbers, xoshiro256+ is even faster.

    The state must be seeded so that it is not everywhere zero. If you have
    a 64-bit seed, we suggest to seed a splitmix64 generator and use its
    output to fill s. */

static inline uint64_t xoshiro256pp_rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoshiro256pp_state[4];

static inline uint64_t next(void) {
    const uint64_t result = xoshiro256pp_rotl(xoshiro256pp_state[0] + xoshiro256pp_state[3], 23) + xoshiro256pp_state[0];

    const uint64_t t = xoshiro256pp_state[1] << 17;

    xoshiro256pp_state[2] ^= xoshiro256pp_state[0];
    xoshiro256pp_state[3] ^= xoshiro256pp_state[1];
    xoshiro256pp_state[1] ^= xoshiro256pp_state[2];
    xoshiro256pp_state[0] ^= xoshiro256pp_state[3];

    xoshiro256pp_state[2] ^= t;

    xoshiro256pp_state[3] = xoshiro256pp_rotl(xoshiro256pp_state[3], 45);

    return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void xoshiro256pp_jump(void) {
    static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xoshiro256pp_state[0];
                s1 ^= xoshiro256pp_state[1];
                s2 ^= xoshiro256pp_state[2];
                s3 ^= xoshiro256pp_state[3];
            }
            next();
        }
        
    xoshiro256pp_state[0] = s0;
    xoshiro256pp_state[1] = s1;
    xoshiro256pp_state[2] = s2;
    xoshiro256pp_state[3] = s3;
}

/* This is the long-jump function for the generator. It is equivalent to
   2^192 calls to next(); it can be used to generate 2^64 starting points,
   from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void xoshiro256pp_longjump(void) {
    static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (LONG_JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xoshiro256pp_state[0];
                s1 ^= xoshiro256pp_state[1];
                s2 ^= xoshiro256pp_state[2];
                s3 ^= xoshiro256pp_state[3];
            }
            next();
        }
        
    xoshiro256pp_state[0] = s0;
    xoshiro256pp_state[1] = s1;
    xoshiro256pp_state[2] = s2;
    xoshiro256pp_state[3] = s3;
}

/* **********************************************************************
 * xoshiro256+
 * ********************************************************************** */

/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.

    See <http://creativecommons.org/publicdomain/zero/1.0/>

    This is xoshiro256+ 1.0, our best and fastest generator for floating-point
    numbers. We suggest to use its upper bits for floating-point
    generation, as it is slightly faster than xoshiro256++/xoshiro256**. It
    passes all tests we are aware of except for the lowest three bits,
    which might fail linearity tests (and just those), so if low linear
    complexity is not considered an issue (as it is usually the case) it
    can be used to generate 64-bit outputs, too.

    We suggest to use a sign test to extract a random Boolean value, and
    right shifts to extract subsets of bits.

    The state must be seeded so that it is not everywhere zero. If you have
    a 64-bit seed, we suggest to seed a splitmix64 generator and use its
    output to fill s. */


static inline uint64_t xoshiro256p_rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}


static uint64_t xoshiro256p_state[4];

static inline uint64_t xoshiro256p_next(void) {
    const uint64_t result = xoshiro256p_state[0] + xoshiro256p_state[3];

    const uint64_t t = xoshiro256p_state[1] << 17;

    xoshiro256p_state[2] ^= xoshiro256p_state[0];
    xoshiro256p_state[3] ^= xoshiro256p_state[1];
    xoshiro256p_state[1] ^= xoshiro256p_state[2];
    xoshiro256p_state[0] ^= xoshiro256p_state[3];

    xoshiro256p_state[2] ^= t;

    xoshiro256p_state[3] = xoshiro256p_rotl(xoshiro256p_state[3], 45);

    return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void xoshiro256p_jump(void) {
    static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xoshiro256p_state[0];
                s1 ^= xoshiro256p_state[1];
                s2 ^= xoshiro256p_state[2];
                s3 ^= xoshiro256p_state[3];
            }
            next();
        }
        
    xoshiro256p_state[0] = s0;
    xoshiro256p_state[1] = s1;
    xoshiro256p_state[2] = s2;
    xoshiro256p_state[3] = s3;
}


/* This is the long-jump function for the generator. It is equivalent to
   2^192 calls to next(); it can be used to generate 2^64 starting points,
   from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void xoshiro256p_longjump(void) {
    static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
        for(int b = 0; b < 64; b++) {
            if (LONG_JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xoshiro256p_state[0];
                s1 ^= xoshiro256p_state[1];
                s2 ^= xoshiro256p_state[2];
                s3 ^= xoshiro256p_state[3];
            }
            next();
        }
        
    xoshiro256p_state[0] = s0;
    xoshiro256p_state[1] = s1;
    xoshiro256p_state[2] = s2;
    xoshiro256p_state[3] = s3;
}

/* **********************************************************************
 * Public interface
 * ********************************************************************** */

/** Generate a random double on the interval [0.0,1.0] */
double random_double(void) {
    uint64_t x = xoshiro256p_next();
    
    return (double) (x >> 11) * 0x1.0p-53;
}

/** Generate a random 32 bit unsigned int */
unsigned int random_int(void) {
    uint64_t x = xoshiro256p_next();
    
    return (unsigned int) (x>>32);
}

/** Initialize the random number generator */
void random_initialize(void) {
    FILE *urandom;
    uint64_t seed = (uint64_t) time(NULL);
    char bytes[sizeof(uint64_t)];
    
    /* Initialize from OS random bits */
    urandom=fopen("/dev/urandom", "r");
    if (urandom) {
        for(int i=0;i<sizeof(unsigned long);i++) bytes[i]=(char) fgetc(urandom);
        seed = *((uint64_t *) bytes);
        
        fclose(urandom);
    } else fprintf(stderr, "Warning: initializing random number generator using time-not recommended for production runs.\n");
    
    /* Use this to initialize splitmix64 */
    splitmix64_seed(seed);
    /* Then initialize xoshiro256pp */
    xoshiro256pp_state[0]=splitmix64_next();
    xoshiro256pp_state[1]=splitmix64_next();
    xoshiro256pp_state[2]=splitmix64_next();
    xoshiro256pp_state[3]=splitmix64_next();
    
    /* ... and xoshiro256p */
    xoshiro256p_state[0]=splitmix64_next();
    xoshiro256p_state[1]=splitmix64_next();
    xoshiro256p_state[2]=splitmix64_next();
    xoshiro256p_state[3]=splitmix64_next();
}