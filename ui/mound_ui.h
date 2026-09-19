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
    _latch { false },
    _presses { 0 },
    _recording { false },
    _loop_count { 0 },
    _loop_length { 0 },
    _loop_tick { 0 }
     {
        _voices.fill({ -1, false, 0 });
     }

    ~MoundUI() {}

    void Init(daisy::DaisySeed& hw);
    void Process(daisy::DaisySeed& hw);

private:
    void _on_pad_touch(uint16_t pad);
    void _on_pad_release(uint16_t pad);
    void _start(uint8_t index);
    void _stop(uint8_t voice);
    void _strike(uint8_t index);
    uint16_t _bar(const uint16_t hold);

    Touch& _touch;
    Mound& _mound;

    // S32 and S33 change job with the mode; these hold and pick up.
    MValue _drum_tone;
    MValue _drum_decay;
    MValue _voice_spread;
    MValue _voice_swell;
    MValue _drum_level;
    MValue _voice_level;

    float _light_box;
    uint8_t _led_ticks;
    bool _voice;
    bool _latch;

    // Which voice pad holds each voice, -1 for none, and its press order.
    struct Voice {
        int8_t pad;
        bool sounding;
        uint16_t age;
    };
    std::array<Voice, kVoiceCount> _voices;
    uint16_t _presses;

    // Drum loop: strikes recorded against the control tick while P10 is
    // held, the hold itself sets the bar.
    struct Step {
        uint16_t tick;
        uint8_t drum;
    };
    bool _recording;
    std::array<Step, kLoopSteps> _loop;
    uint8_t _loop_count;
    uint16_t _loop_length;
    uint16_t _loop_tick;
};

};
