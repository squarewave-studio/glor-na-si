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

    _touch.Process();

    // Walk, the left fader, so the right hand stays on the pads ...
    _mound.SetWalk(_touch.knobs().s36().Process());

    // Light box (reserved), the right fader ......................
    _light_box = _touch.knobs().s37().Process();

    // Loop speed, in either mode, as played at centre ..............
    _loop_rate = exp2f(daisysp::fmap(_touch.knobs().s30().Process(), -kLoopSpeedOctaves, kLoopSpeedOctaves));

    auto pitch    = _touch.knobs().s31().Process();
    auto tone     = _touch.knobs().s32().Process();
    auto time     = _touch.knobs().s33().Process();
    auto unsteady = _touch.knobs().s34().Process();
    auto level    = _touch.knobs().s35().Process();
    // S31 is the drum tuning in DRUM and the voice pitch in VOICE,
    // S34 the hardness in DRUM and the unsteadiness in VOICE, S32 and
    // S33 the drum tone and decay or the voice spread and swell. The
    // inactive job holds, and picks up when the knob comes back to
    // where it left it ........................................
    _mound.SetDrumTune(_drum_tune.Process(pitch, !_voice));
    _mound.SetPitch(_voice_pitch.Process(pitch, _voice));
    _mound.SetDrumHardness(_drum_hardness.Process(unsteady, !_voice));
    _mound.SetUnsteadiness(_voice_unsteady.Process(unsteady, _voice));
    _mound.SetDrumTone(_drum_tone.Process(tone, !_voice));
    _mound.SetDrumDecay(_drum_decay.Process(time, !_voice));
    _mound.SetSpread(_voice_spread.Process(tone, _voice));
    _mound.SetSwell(_voice_swell.Process(time, _voice));

    // S35 is the level of whichever mode is on, held and picked up
    // the same way .............................................
    _mound.SetDrumLevel(_drum_level.Process(level, !_voice));
    _mound.SetVoiceLevel(_voice_level.Process(level, _voice));

    bool singing = false;
    for (uint8_t v = 0; v < kVoiceCount; v++) singing |= _voices[v].sounding;

    // The loop, in either mode ...................................
    if (_recording) {
        _loop_tick++;
        if (_loop_tick - _last_step >= kLoopIdleTicks || _loop_tick >= 60000) _end_take(true);
    } else if (_loop_length > 0) {
        auto from = _loop_phase;
        auto to = from + _loop_rate;
        _play(from, to);
        if (to >= (float)_loop_length) {
            to -= (float)_loop_length;
            _play(0.f, to);
        }
        _loop_phase = to;
    }

    // LED flashes on a strike, and while recording stays on and
    // blinks off for everything it takes. Singing in VOICE ........
    if (_led_ticks > 0) _led_ticks--;
    auto flash = _led_ticks > 0;
    hw.SetLed(_voice ? singing : (_recording ? !flash : flash));
};

// P10 takes and P11 clears, in either mode. Tap P10, and what you play
// until the next tap comes round again from the first thing you played:
// drum hits, and vowels, each where you stood. A vowel still down when
// the take ends is the loop's from then on and keeps singing when the
// pad comes up. A take with nothing played for a while ends itself.
void MoundUI::_on_pad_touch(uint16_t pad) {
    if (pad == kLoopClearPad) {
        _recording = false;
        _loop_count = 0;
        _loop_length = 0;
        for (uint8_t v = 0; v < kVoiceCount; v++) {
            if (_voices[v].held) _stop(v);
            _voices[v].recorded = false;
        }
        return;
    }
    if (pad == kLoopRecordPad) {
        if (_recording) _end_take(false);
        else _begin_take();
        return;
    }
    if (_voice) {
        for (uint8_t i = 0; i < kVoicePadCount; i++) {
            if (kVoicePads[i].pad != pad) continue;
            auto at = _mound.Listener();
            if (_recording) _record(SING, i, at);
            _start(i, at, false);
            return;
        }
        return;
    }
    for (uint8_t i = 0; i < kDrumPadCount; i++) {
        if (kDrumPads[i] != pad) continue;
        auto at = _mound.Listener();
        if (_recording) _record(STRIKE, i, at);
        _strike(i, at);
        return;
    }
};

void MoundUI::_on_pad_release(uint16_t pad) {
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].held || _voices[v].pad < 0 || kVoicePads[_voices[v].pad].pad != pad) continue;
        if (_recording && _voices[v].recorded) _record(REST, _voices[v].pad, _mound.Listener());
        _stop(v);
    }
};

// A new take replaces the loop and lets go of what it held. Vowels
// already under your fingers count as pressed at the start.
void MoundUI::_begin_take() {
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].held) _stop(v);
    }
    _recording = true;
    _loop_count = 0;
    _loop_length = 0;
    _loop_tick = 0;
    _last_step = 0;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (!_voices[v].sounding || _voices[v].pad < 0) continue;
        _voices[v].recorded = true;
        _record(SING, _voices[v].pad, _voices[v].at);
    }
};

// Nothing played, nothing loops. A vowel taken during the take and
// still down is the loop's now. A take that ended itself is trimmed,
// one ended by a tap is exactly as long as the tap made it.
void MoundUI::_end_take(bool trim) {
    _recording = false;
    _loop_length = _loop_count > 0 ? _bar(_loop_tick, trim) : 0;
    _loop_tick = 0;
    _loop_phase = 0.f;
    for (auto& v : _voices) {
        if (_loop_length > 0 && v.recorded && v.sounding && v.pad >= 0) v.held = true;
        v.recorded = false;
    }
};

void MoundUI::_record(Kind kind, uint8_t index, float at) {
    if (_loop_count >= kLoopSteps) return;
    _loop[_loop_count++] = { _loop_tick, kind, index, at };
    _last_step = _loop_tick;
    _led_ticks = kLedStrikeTicks;
};

// The loop plays a hit where it was struck. A vowel it starts is its to
// hold. One it started last time round and never ended just goes on
// singing rather than starting again, and it never takes a vowel from
// under your fingers.
// Every step from one tick up to the next, at whatever speed S30 says.
void MoundUI::_play(const float from, const float to) {
    for (uint8_t i = 0; i < _loop_count; i++) {
        auto tick = (float)_loop[i].tick;
        if (tick >= from && tick < to) _replay(_loop[i]);
    }
};

void MoundUI::_replay(const Step& step) {
    switch (step.kind) {
    case STRIKE:
        _strike(step.index, step.at);
        break;
    case SING: {
        bool free = false;
        for (auto& v : _voices) {
            if (v.held && v.sounding && v.pad == step.index) return;
            free |= !v.sounding || v.held;
        }
        if (free) _start(step.index, step.at, true);
        break;
    }
    case REST:
        for (uint8_t v = 0; v < kVoiceCount; v++) {
            if (_voices[v].held && _voices[v].pad == step.index) _stop(v);
        }
        break;
    }
};

void MoundUI::_strike(uint8_t index, float at) {
    _mound.Strike(index, at);
    _led_ticks = kLedStrikeTicks;
};

// The bar. Steps shift so the first sits at 0. Ended by a tap, the
// length is the tap, rests and all. Ended by itself, the length is the
// last step plus the median gap between steps, so the wait it took to
// give up is not in the bar; a single hit then nothing is no loop, a
// single vowel still held is a drone.
uint16_t MoundUI::_bar(const uint16_t hold, const bool trim) {
    auto lead = _loop[0].tick;
    for (uint8_t i = 0; i < _loop_count; i++) _loop[i].tick -= lead;
    uint16_t length = hold - lead;
    if (trim && _loop_count == 1 && _loop[0].kind == STRIKE) return 0;
    if (trim && _loop_count > 1) {
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
    if (length < kLoopMinTicks) length = kLoopMinTicks;
    for (uint8_t i = 0; i < _loop_count; i++) {
        if (_loop[i].tick >= length) _loop[i].tick = length - 1;
    }
    return length;
};

// A pad takes a silent voice first, then one the loop holds, then the
// older of the ones under your fingers. Gate off then on so a stolen
// voice starts a new syllable.
void MoundUI::_start(uint8_t index, float at, bool held) {
    uint8_t pick = 0;
    uint8_t best = 3;
    uint16_t oldest = 0xffff;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        uint8_t rank = !_voices[v].sounding ? 0 : (_voices[v].held ? 1 : 2);
        if (rank < best || (rank == best && _voices[v].age < oldest)) {
            pick = v;
            best = rank;
            oldest = _voices[v].age;
        }
    }
    _voices[pick] = { (int8_t)index, true, held, !held && _recording, ++_presses, at };
    _mound.SetVowel(pick, kVoicePads[index].vowel);
    _mound.SetInterval(pick, kVoicePads[index].semitones);
    _mound.SetSing(pick, false);
    _mound.Sing(pick, at);
};

void MoundUI::_stop(uint8_t voice) {
    _mound.SetSing(voice, false);
    _voices[voice].pad = -1;
    _voices[voice].sounding = false;
    _voices[voice].held = false;
    _voices[voice].recorded = false;
};
