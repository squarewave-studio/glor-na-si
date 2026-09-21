/*
Copyright (c) 2023 Electrosmith, Corp, Sean Costello, Istvan Varga, Paul Batchelor

Use of this source code is governed by the LGPL V2.1
license that can be found in the LICENSE file or at
https://opensource.org/license/lgpl-2-1/
*/

#pragma once

// Glór na Sí: this is DaisySP-LGPL's ReverbSc (DaisySP 599511b, 28 May 2025),
// Sean Costello's reverb as ported by Istvan Varga and Paul Batchelor,
// copied here unchanged in what it computes. Three things differ: it
// lives in namespace synthux as Reverb; its delay lines are laid out in
// samples, not bytes, and the buffer is sized for the eight lines at
// 48 kHz, 98 KB in place of 396; and Init fails cleanly if a sample rate
// needs more. The licence above still applies to this file.

#include <cstddef>

namespace synthux
{

// The eight lines' delays in samples at 48 kHz, as Costello set them,
// with the room the pitch modulation needs and the library's own
// 16-sample margin. The buffer holds exactly that.
static constexpr float kReverbSampleRate = 48000.f;
static constexpr float kReverbDelays[8] = { 2473.f, 2767.f, 3217.f, 3557.f, 3907.f, 4127.f, 2143.f, 1933.f };
static constexpr float kReverbWander[8] = { 0.0010f, 0.0011f, 0.0017f, 0.0006f, 0.0010f, 0.0011f, 0.0017f, 0.0006f };
static constexpr int ReverbLineSamples(const int n, const float sr) {
  return (int)((kReverbDelays[n] / kReverbSampleRate + kReverbWander[n] * 1.f * 1.125f) * sr + 16.5f);
}
static constexpr size_t ReverbBufferSize() {
  size_t total = 0;
  for (int n = 0; n < 8; n++) total += (size_t)ReverbLineSamples(n, kReverbSampleRate);
  return total;
}
static constexpr size_t kReverbMaxSize = ReverbBufferSize();
/**Delay line for internal reverb use
*/
typedef struct
{
    int    write_pos;         /**< write position */
    int    buffer_size;       /**< buffer size */
    int    read_pos;          /**< read position */
    int    read_pos_frac;     /**< fractional component of read pos */
    int    read_pos_frac_inc; /**< increment for fractional */
    int    dummy;             /**<  dummy var */
    int    seed_val;          /**< randseed */
    int    rand_line_cnt;     /**< number of random lines */
    float  filter_state;      /**< state of filter */
    float *buf;               /**< buffer ptr */
} ReverbDl;

/** Stereo Reverb */
class Reverb
{
  public:
    Reverb() {}
    ~Reverb() {}
    /** Initializes the reverb module, and sets the sample_rate at which the Process function will be called.
        Returns 0 if all good, or 1 if it runs out of delay times exceed maximum allowed.
    */
    int Init(float sample_rate);

    /** Process the input through the reverb, and updates values of out1, and out2 with the new processed signal.
    */
    int Process(const float &in1, const float &in2, float *out1, float *out2);

    /** controls the reverb time. reverb tail becomes infinite when set to 1.0
        \param fb - sets reverb time. range: 0.0 to 1.0
    */
    inline void SetFeedback(const float &fb) { feedback_ = fb; }
    /** controls the internal dampening filter's cutoff frequency.
        \param freq - low pass frequency. range: 0.0 to sample_rate / 2
    */
    inline void SetLpFreq(const float &freq) { lpfreq_ = freq; }

  private:
    void       NextRandomLineseg(ReverbDl *lp, int n);
    int        InitDelayLine(ReverbDl *lp, int n);
    float      feedback_, lpfreq_;
    float      i_sample_rate_, i_pitch_mod_, i_skip_init_;
    float      sample_rate_;
    float      damp_fact_;
    float      prv_lpfreq_;
    int        init_done_;
    ReverbDl   delay_lines_[8];
    float      aux_[kReverbMaxSize];
};


} // namespace synthux
