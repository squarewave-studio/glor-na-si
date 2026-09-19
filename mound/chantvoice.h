#ifndef CHANTVOICE_H
#define CHANTVOICE_H

#include <stdint.h>
#include "chant_profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHANT_TABLE 512
#define CHANT_BLOCK 480      /* 10 ms at 48k; call chant_block() every CHANT_BLOCK samples */

typedef struct {
    float sr;
    float f0;               /* hz, set before or during a note */
    float detune;           /* cents, constant per instance */
    float vowel;            /* 0..CHANT_NPROFILES-1, fractional, morph position while singing */
    float on_time;          /* seconds, opening morph and swell */
    float off_time;         /* seconds, closing morph and fade */
    float wander_shared;    /* dB */
    float wander_ind;       /* dB */
    float wander_rate;      /* hz */
    float drift_cents;
    float drift_rate;       /* hz */
    float jitter;           /* fraction of period, per cycle */
    float shimmer;          /* fraction of amplitude, per cycle */
    float tilt;             /* dB per octave of extra darkening at the edges of a swell */

    /* state */
    int gate;
    float t_edge;           /* seconds since the last gate change */
    float env;
    float morph;            /* 0 closed .. 1 open */
    float shared_w, ind_w[CHANT_NH], drift_w;
    float a_shared, a_drift;
    float norm_shared, norm_drift;
    float fbase, phase, cyc_f, cyc_a, jit, shim;
    float tbl_cur[CHANT_TABLE], tbl_next[CHANT_TABLE];
    float amp_cur[CHANT_NH];
    int pos;
    uint32_t rng;
} chant_voice;

void  chant_init(chant_voice *v, float sr, uint32_t seed);
void  chant_gate(chant_voice *v, int on);
void  chant_block(chant_voice *v);          /* once per CHANT_BLOCK samples, before the samples */
float chant_process(chant_voice *v);        /* one sample */

#ifdef __cplusplus
}
#endif

#endif
