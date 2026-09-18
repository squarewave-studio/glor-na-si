#include "tomb_ui.h"
#include "config.h"
#include "log.h"

using namespace synthux;
using namespace daisy;

void TombUI::Init(daisy::DaisySeed& hw) {
    // Callbacks ................................................
    using namespace std::placeholders;
    auto on_touch = std::bind(&TombUI::_on_pad_touch, this, _1);
    auto on_release = std::bind(&TombUI::_on_pad_release, this, _1);
    _touch.pads().SetOnTouch(on_touch);
    _touch.pads().SetOnRelease(on_release);
};

void TombUI::Process(DaisySeed& hw) {
    _touch.Process();

    // Walk (real-time) ...........................................
    _tomb.SetWalk(_touch.knobs().s37().Process());

    // Drum (sample-and-hold at strike time) ......................
    _tomb.SetDrumTone(_touch.knobs().s32().Process());
    _tomb.SetDrumDecay(_touch.knobs().s33().Process());

    // Light box (reserved) .......................................
    _light_box = _touch.knobs().s36().Process();

    if (_led_ticks > 0) _led_ticks--;
    hw.SetLed(_led_ticks > 0);
};

void TombUI::_on_pad_touch(uint16_t pad) {
    for (uint8_t i = 0; i < kDrumPadCount; i++) {
        if (kDrumPads[i] != pad) continue;
        _tomb.Strike(i);
        _led_ticks = kLedStrikeTicks;
        #if DEBUG
        LOGINT(pad);
        #endif
        return;
    }
};

void TombUI::_on_pad_release(uint16_t pad) {};
