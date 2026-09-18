#pragma once

#include "daisy_seed.h"
#include "../touch/touch.h"
#include "../tomb/tomb.h"
#include "config.h"
#include <functional>

namespace synthux {

class TombUI {
public:
    TombUI(Touch& touch, Tomb& tomb):
    _touch { touch },
    _tomb { tomb },
    _light_box { 0.f },
    _led_ticks { 0 }
     {}

    ~TombUI() {}

    void Init(daisy::DaisySeed& hw);
    void Process(daisy::DaisySeed& hw);

private:
    void _on_pad_touch(uint16_t pad);
    void _on_pad_release(uint16_t pad);

    Touch& _touch;
    Tomb& _tomb;

    float _light_box;
    uint8_t _led_ticks;
};

};
