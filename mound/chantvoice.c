#include <math.h>
#include <string.h>
#include "chantvoice.h"

static float costab[CHANT_TABLE];
static int costab_ready = 0;

static uint32_t xorshift(uint32_t *s)
{
    uint32_t x = *s;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return *s = x;
}

static float frand(uint32_t *s)
{
    return (xorshift(s) & 0xffffff) * (1.0f / 8388608.0f) - 1.0f;
}

/* roughly gaussian, unit variance */
static float nrand(uint32_t *s)
{
    return frand(s) + frand(s) + frand(s);
}

static float ease(float x)
{
    if (x < 0) x = 0;
    if (x > 1) x = 1;
    return x * x * (3 - 2 * x);
}

static float onepole_coef(float rate_hz, float sr)
{
    return 1.0f - expf(-2.0f * (float)M_PI * rate_hz * CHANT_BLOCK / sr);
}

void chant_init(chant_voice *v, float sr, uint32_t seed)
{
    if (!costab_ready) {
        for (int i = 0; i < CHANT_TABLE; i++)
            costab[i] = cosf(2.0f * (float)M_PI * i / CHANT_TABLE);
        costab_ready = 1;
    }
    memset(v, 0, sizeof *v);
    v->sr = sr;
    v->f0 = 65.9f;
    v->vowel = CHANT_AAH;
    v->on_time = 1.4f;
    v->off_time = 1.8f;
    v->wander_shared = 3.5f;
    v->wander_ind = 4.0f;
    v->wander_rate = 0.3f;
    v->drift_cents = 25.0f;
    v->drift_rate = 0.15f;
    v->jitter = 0.006f;
    v->shimmer = 0.03f;
    v->tilt = 0.0f;
    v->rng = seed ? seed : 0x9e3779b9u;
    v->cyc_f = 1.0f;
    v->cyc_a = 1.0f;
    v->t_edge = 100.0f;
    v->a_shared = onepole_coef(v->wander_rate, sr);
    v->a_drift = onepole_coef(v->drift_rate, sr);
    v->norm_shared = sqrtf(v->a_shared / (2.0f - v->a_shared));
    v->norm_drift = sqrtf(v->a_drift / (2.0f - v->a_drift));
    for (int i = 0; i < 600; i++) {
        v->shared_w += v->a_shared * (nrand(&v->rng) - v->shared_w);
        v->drift_w += v->a_drift * (nrand(&v->rng) - v->drift_w);
        for (int h = 0; h < CHANT_NH; h++)
            v->ind_w[h] += v->a_shared * (nrand(&v->rng) - v->ind_w[h]);
    }
}

void chant_gate(chant_voice *v, int on)
{
    on = on ? 1 : 0;
    if (on == v->gate) return;
    v->gate = on;
    v->t_edge = 0;
}

void chant_block(chant_voice *v)
{
    const float dt = (float)CHANT_BLOCK / v->sr;
    const float *closed = chant_profiles[CHANT_MMM];
    float prof[CHANT_NH];

    v->t_edge += dt;
    if (v->gate) {
        v->morph = ease(v->t_edge / v->on_time);
        v->env += (1.0f - v->env) * dt / (0.45f * v->on_time);
    } else {
        v->morph = 1.0f - ease(v->t_edge / v->off_time);
        v->env += (0.0f - v->env) * dt / (0.45f * v->off_time);
    }

    float vw = v->vowel;
    if (vw < 0) vw = 0;
    if (vw > CHANT_NPROFILES - 1) vw = CHANT_NPROFILES - 1;
    int i0 = (int)vw;
    int i1 = i0 + 1 < CHANT_NPROFILES ? i0 + 1 : i0;
    float fr = vw - i0;

    v->shared_w += v->a_shared * (nrand(&v->rng) - v->shared_w);
    v->drift_w += v->a_drift * (nrand(&v->rng) - v->drift_w);
    float sh = v->wander_shared * v->shared_w / v->norm_shared;
    float cents = v->drift_cents * v->drift_w / v->norm_drift + v->detune;
    v->fbase = v->f0 * exp2f(cents * (1.0f / 1200.0f));
    float dark = v->tilt * (1.0f - v->env);

    for (int h = 0; h < CHANT_NH; h++) {
        float open = chant_profiles[i0][h] + (chant_profiles[i1][h] - chant_profiles[i0][h]) * fr;
        float db = closed[h] + (open - closed[h]) * v->morph;
        v->ind_w[h] += v->a_shared * (nrand(&v->rng) - v->ind_w[h]);
        db += sh + v->wander_ind * v->ind_w[h] / v->norm_shared;
        if (dark > 0) db -= dark * log2f((float)(h + 1));
        prof[h] = db < -70.0f ? 0.0f : exp2f(db * 0.1660964f);
    }

    memcpy(v->tbl_cur, v->tbl_next, sizeof v->tbl_cur);
    memset(v->tbl_next, 0, sizeof v->tbl_next);
    for (int h = 0; h < CHANT_NH; h++) {
        float a = prof[h];
        if (a == 0.0f) continue;
        int step = h + 1, k = 0;
        for (int i = 0; i < CHANT_TABLE; i++) {
            v->tbl_next[i] += a * costab[k];
            k += step;
            if (k >= CHANT_TABLE) k -= CHANT_TABLE;
        }
    }
    v->pos = 0;
}

float chant_process(chant_voice *v)
{
    v->phase += v->fbase * v->cyc_f / v->sr;
    if (v->phase >= 1.0f) {
        v->phase -= 1.0f;
        v->jit = 0.5f * v->jit + 0.5f * frand(&v->rng);
        v->shim = 0.5f * v->shim + 0.5f * frand(&v->rng);
        v->cyc_f = 1.0f + v->jitter * v->jit;
        v->cyc_a = 1.0f + v->shimmer * v->shim;
    }

    float p = v->phase * CHANT_TABLE;
    int j = (int)p;
    float fr = p - j;
    int j1 = j + 1 < CHANT_TABLE ? j + 1 : 0;
    float s0 = v->tbl_cur[j] + (v->tbl_cur[j1] - v->tbl_cur[j]) * fr;
    float s1 = v->tbl_next[j] + (v->tbl_next[j1] - v->tbl_next[j]) * fr;
    float xf = (float)v->pos * (1.0f / CHANT_BLOCK);
    if (v->pos < CHANT_BLOCK - 1) v->pos++;

    return (s0 + (s1 - s0) * xf) * v->cyc_a * v->env * v->env;
}
