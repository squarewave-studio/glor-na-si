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
    _started { false },
    _presses { 0 },
    _last_sound { SOUND_AAH },
    _loop_rate { 1.f }
     {
        _voices.fill({ -1, false, false, false, 0, 0.f, SOUND_AAH });
     }

    ~MoundUI() {}

    void Init(daisy::DaisySeed& hw);
    void Process(daisy::DaisySeed& hw);

private:
    // What a loop holds: a drum hit, a vowel starting, a vowel ending.
    enum Kind : uint8_t { STRIKE, SING, REST };
    struct Step {
        uint16_t tick;
        Kind kind;
        uint8_t index;   // drum pad index, or voice pad index
        float at;        // metres, where the listener stood
        uint8_t sound;   // for a vowel, the sound it was sung with
    };

    // A loop: what you did between two taps of P10 in one mode, against
    // the control tick. Every take sets the bar, and a take while a loop
    // runs replaces it. One for the drum, one for the voices, each with
    // its own bar, both playing in either mode.
    struct Loop {
        bool recording = false;
        uint16_t idle = 0;         // ticks since the last thing taken
        uint16_t tick = 0;         // a take counts here
        uint16_t length = 0;       // the bar, 0 for no loop
        float phase = 0.f;         // playback position in ticks
        std::array<Step, kLoopSteps> steps;
        uint8_t count = 0;
    };

    void _on_pad_touch(uint16_t pad);
    void _on_pad_release(uint16_t pad);
    void _start(uint8_t index, float at, bool held, bool recorded, uint8_t sound);
    void _stop(uint8_t voice);
    void _strike(uint8_t index, float at);
    void _clear(Loop& loop);
    void _begin_take(Loop& loop);
    void _end_take(Loop& loop, bool trim);
    bool _record(Loop& loop, Kind kind, uint8_t index, float at, uint8_t sound = 0);
    uint8_t _sound_for(uint8_t index);
    void _play(Loop& loop, const float from, const float to);
    void _replay(const Step& step);
    uint16_t _bar(Loop& loop, const uint16_t hold, const bool trim);
    Loop& _loop() { return _loops[_voice ? 1 : 0]; }

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
    bool _started;

    // Each voice: the voice pad it sings, -1 for none; whether it
    // sounds; whether the voice loop holds it, so its pad coming up
    // means nothing; whether it was taken during a take; press order;
    // where it stands.
    struct Voice {
        int8_t pad;
        bool sounding;
        bool held;
        bool recorded;
        uint16_t age;
        float at;
        uint8_t sound;
    };
    std::array<Voice, kVoiceCount> _voices;
    uint16_t _presses;
    uint8_t _last_sound;          // what the front row sang last, for the other pads

    std::array<Loop, 2> _loops;   // drum, voice
    float _loop_rate;             // ticks per control tick, S30, both loops
};

};
