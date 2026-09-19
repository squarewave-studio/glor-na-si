#ifndef CHANT_PROFILES_H
#define CHANT_PROFILES_H

#define CHANT_NH 24
#define CHANT_NPROFILES 6

enum { CHANT_MMM, CHANT_OOH, CHANT_OH, CHANT_AAH, CHANT_AH, CHANT_OVERTONE };

/* harmonic levels in dB relative to the fundamental, h1..h24, measured from the reference chant at C2 */
static const float chant_profiles[CHANT_NPROFILES][CHANT_NH] = {
    { 0.0f, -7.4f, -27.3f, -11.0f, -29.1f, -31.6f, -40.4f, -41.0f, -46.7f, -46.9f, -43.5f, -50.8f, -54.0f, -52.7f, -48.4f, -50.2f, -53.6f, -54.6f, -54.8f, -48.0f, -56.6f, -60.1f, -58.7f, -55.7f },  /* MMM */
    { 0.0f, -10.3f, -24.0f, -12.7f, -27.2f, -23.1f, -21.3f, -27.6f, -34.1f, -36.5f, -33.0f, -37.9f, -46.4f, -50.7f, -57.8f, -57.4f, -59.1f, -60.2f, -58.8f, -54.2f, -57.7f, -59.5f, -57.3f, -56.3f },  /* OOH */
    { 0.0f, -6.8f, -20.5f, -14.4f, -23.7f, -19.6f, -17.8f, -24.1f, -29.1f, -31.5f, -28.0f, -32.9f, -41.4f, -45.7f, -52.8f, -53.8f, -54.8f, -56.9f, -57.1f, -53.2f, -56.5f, -57.8f, -54.8f, -53.9f },  /* OH */
    { 0.0f, -3.3f, -17.0f, -16.2f, -20.2f, -16.1f, -14.3f, -20.6f, -24.1f, -26.5f, -23.0f, -27.9f, -36.4f, -40.7f, -47.8f, -50.2f, -50.5f, -53.7f, -55.4f, -52.3f, -55.4f, -56.2f, -52.4f, -51.5f },  /* AAH */
    { 0.0f, -3.3f, -17.0f, -16.2f, -20.2f, -16.1f, -14.3f, -20.3f, -9.0f, -19.5f, -23.0f, -27.9f, -36.4f, -39.1f, -34.4f, -33.1f, -45.3f, -53.7f, -55.4f, -52.3f, -55.4f, -56.2f, -52.4f, -51.5f },  /* AH */
    { 0.0f, -10.3f, -24.0f, -12.7f, -27.2f, -23.1f, -21.3f, -19.6f, -12.1f, -28.5f, -33.0f, -37.9f, -46.4f, -50.7f, -57.8f, -57.4f, -59.1f, -60.2f, -58.8f, -54.2f, -57.7f, -59.5f, -57.3f, -56.3f },  /* OVERTONE */
};

#endif
