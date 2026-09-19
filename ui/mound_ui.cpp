#include "mound_ui.h"
#include "config.h"
#include "log.h"

using namespace synthux;
using namespace daisy;

void MoundUI::Init(daisy::DaisySeed& hw) {
    // Callbacks ................................................
    using namespace std::placeholders;
    auto on_touch = std::bind(&MoundUI::_on_pad_touch, this, _1);
    auto on_release = std::bind(&MoundUI::_on_pad_release, this, _1);
    _touch.pads().SetOnTouch(on_touch);
    _touch.pads().SetOnRelease(on_release);
};

void MoundUI::Process(DaisySeed& hw) {
    // Mode: switch A up is VOICE, anything else is DRUM ..........
    _voice = _touch.switches().A() == Switch3::POS_UP;

    // Latch: switch B up keeps the note after the pads are up, and
    // across a switch to DRUM so the drum can play over it .......
    _latch = _touch.switches().B() == Switch3::POS_UP;

    _touch.Process();

    // Walk (real-time) ...........................................
    _mound.SetWalk(_touch.knobs().s37().Process());

    // Light box (reserved) .......................................
    _light_box = _touch.knobs().s36().Process();

    auto pitch    = _touch.knobs().s31().Process();
    auto tone     = _touch.knobs().s32().Process();
    auto time     = _touch.knobs().s33().Process();
    auto unsteady = _touch.knobs().s34().Process();
    auto level    = _touch.knobs().s35().Process();
    // Voice pitch and unsteadiness, in either mode ...............
    _mound.SetPitch(pitch);
    _mound.SetUnsteadiness(unsteady);

    // Drum tone and decay, voice spread and swell, share S32 and
    // S33. The inactive pair holds, and picks up when the knob
    // comes back to where it left that pair ......................
    _mound.SetDrumTone(_drum_tone.Process(tone, !_voice));
    _mound.SetDrumDecay(_drum_decay.Process(time, !_voice));
    _mound.SetSpread(_voice_spread.Process(tone, _voice));
    _mound.SetSwell(_voice_swell.Process(time, _voice));

    // S35 is the level of whichever mode is on, held and picked up
    // the same way .............................................
    _mound.SetDrumLevel(_drum_level.Process(level, !_voice));
    _mound.SetVoiceLevel(_voice_level.Process(level, _voice));

    // Latch down lets go of every voice whose pad is up ..........
    bool singing = false;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].sounding && _voices[v].pad < 0 && !_latch) _stop(v);
        singing |= _voices[v].sounding;
    }

    // Drum loop, in either mode ..................................
    if (_recording) {
        _loop_tick++;
    } else if (_loop_length > 0) {
        for (uint8_t i = 0; i < _loop_count; i++) {
            if (_loop[i].tick == _loop_tick) _strike(_loop[i].drum);
        }
        if (++_loop_tick >= _loop_length) _loop_tick = 0;
    }

    // LED flashes on a strike, and while recording stays on and
    // blinks off for every strike it takes. Singing in VOICE ......
    if (_led_ticks > 0) _led_ticks--;
    auto flash = _led_ticks > 0;
    hw.SetLed(_voice ? singing : (_recording ? !flash : flash));
};

void MoundUI::_on_pad_touch(uint16_t pad) {
    if (pad == kLoopClearPad) {
        _recording = false;
        _loop_count = 0;
        _loop_length = 0;
        return;
    }
    if (_voice) {
        for (uint8_t i = 0; i < kVoicePadCount; i++) {
            if (kVoicePads[i].pad != pad) continue;
            _start(i);
            return;
        }
        return;
    }
    if (pad == kLoopRecordPad) {
        _recording = true;
        _loop_count = 0;
        _loop_length = 0;
        _loop_tick = 0;
        return;
    }
    for (uint8_t i = 0; i < kDrumPadCount; i++) {
        if (kDrumPads[i] != pad) continue;
        if (_recording && _loop_count < kLoopSteps) {
            _loop[_loop_count++] = { _loop_tick, i };
        }
        _strike(i);
        #if DEBUG
        LOGINT(pad);
        #endif
        return;
    }
};

void MoundUI::_strike(uint8_t index) {
    _mound.Strike(index);
    _led_ticks = kLedStrikeTicks;
};

// The bar. Strikes shift so the first sits at 0, and the length is the
// hold or, when the hold ran on past the last strike, the last strike
// plus the median gap between strikes, whichever is shorter. A late
// release then leaves no dead air, an early one still cuts the bar.
uint16_t MoundUI::_bar(const uint16_t hold) {
    auto lead = _loop[0].tick;
    for (uint8_t i = 0; i < _loop_count; i++) _loop[i].tick -= lead;
    uint16_t length = hold - lead;
    if (_loop_count > 1) {
        std::array<uint16_t, kLoopSteps> gaps;
        uint8_t count = 0;
        for (uint8_t i = 1; i < _loop_count; i++) {
            uint16_t gap = _loop[i].tick - _loop[i - 1].tick;
            uint8_t j = count++;
            while (j > 0 && gaps[j - 1] > gap) {
                gaps[j] = gaps[j - 1];
                j--;
            }
            gaps[j] = gap;
        }
        uint16_t span = _loop[_loop_count - 1].tick + gaps[count / 2];
        if (span < length) length = span;
    }
    return length > 0 ? length : 1;
};

void MoundUI::_on_pad_release(uint16_t pad) {
    if (pad == kLoopRecordPad && _recording) {
        // Nothing played, nothing loops.
        _recording = false;
        _loop_length = _loop_count > 0 ? _bar(_loop_tick) : 0;
        _loop_tick = 0;
        return;
    }
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].pad < 0 || kVoicePads[_voices[v].pad].pad != pad) continue;
        _voices[v].pad = -1;
        if (!_latch) _stop(v);
    }
};

// A pad takes a silent voice first, then a latched one, then the older
// of the held. Gate off then on so a stolen voice starts a new syllable.
void MoundUI::_start(uint8_t index) {
    uint8_t pick = 0;
    uint8_t best = 3;
    uint16_t oldest = 0xffff;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        uint8_t rank = !_voices[v].sounding ? 0 : (_voices[v].pad < 0 ? 1 : 2);
        if (rank < best || (rank == best && _voices[v].age < oldest)) {
            pick = v;
            best = rank;
            oldest = _voices[v].age;
        }
    }
    _voices[pick] = { (int8_t)index, true, ++_presses };
    _mound.SetVowel(pick, kVoicePads[index].vowel);
    _mound.SetInterval(pick, kVoicePads[index].semitones);
    _mound.SetSing(pick, false);
    _mound.SetSing(pick, true);
};

void MoundUI::_stop(uint8_t voice) {
    _mound.SetSing(voice, false);
    _voices[voice].sounding = false;
};
