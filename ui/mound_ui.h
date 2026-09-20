#pragma once

#include "daisy_seed.h"
#include "../touch/touch.h"
#include "../mound/mound.h"
#include "mvalue.h"
#include "config.h"
#include <functional>

namespace synthux {

class MoundUI {
public:
    MoundUI(Touch& touch, Mound& mound):
    _touch { touch },
    _mound { mound },
    _light_box { 0.f },
    _led_ticks { 0 },
    _voice { false },
    _presses { 0 },
    _recording { false },
    _last_step { 0 },
    _loop_count { 0 },
    _loop_length { 0 },
    _loop_tick { 0 },
    _loop_phase { 0.f },
    _loop_rate { 1.f }
     {
        _voices.fill({ -1, false, false, false, 0, 0.f });
     }

    ~MoundUI() {}

    void Init(daisy::DaisySeed& hw);
    void Process(daisy::DaisySeed& hw);

private:
    // What the loop holds: a drum hit, a vowel starting, a vowel ending.
    enum Kind : uint8_t { STRIKE, SING, REST };
    struct Step {
        uint16_t tick;
        Kind kind;
        uint8_t index;   // drum pad index, or voice pad index
        float at;        // metres, where the listener stood
    };

    void _on_pad_touch(uint16_t pad);
    void _on_pad_release(uint16_t pad);
    void _start(uint8_t index, float at, bool held);
    void _stop(uint8_t voice);
    void _strike(uint8_t index, float at);
    void _begin_take();
    void _end_take(bool trim);
    void _record(Kind kind, uint8_t index, float at);
    void _replay(const Step& step);
    void _play(const float from, const float to);
    uint16_t _bar(const uint16_t hold, const bool trim);

    Touch& _touch;
    Mound& _mound;

    // S31 to S35 change job with the mode; these hold and pick up.
    MValue _drum_tone;
    MValue _drum_decay;
    MValue _voice_spread;
    MValue _voice_swell;
    MValue _drum_level;
    MValue _voice_level;
    MValue _drum_tune;
    MValue _voice_pitch;
    MValue _drum_hardness;
    MValue _voice_unsteady;

    float _light_box;
    uint8_t _led_ticks;
    bool _voice;

    // Each voice: the voice pad it sings, -1 for none; whether it
    // sounds; whether the loop holds it, so its pad coming up means
    // nothing; whether it was taken during a take; press order; where
    // it stands.
    struct Voice {
        int8_t pad;
        bool sounding;
        bool held;
        bool recorded;
        uint16_t age;
        float at;
    };
    std::array<Voice, kVoiceCount> _voices;
    uint16_t _presses;

    // The loop: what you did between two taps of P10, against the
    // control tick. Plays in either mode.
    bool _recording;
    uint16_t _last_step;   // tick of the last thing taken
    std::array<Step, kLoopSteps> _loop;
    uint8_t _loop_count;
    uint16_t _loop_length;
    uint16_t _loop_tick;
    float _loop_phase;     // playback position in ticks
    float _loop_rate;      // ticks per control tick, S30
};

};
