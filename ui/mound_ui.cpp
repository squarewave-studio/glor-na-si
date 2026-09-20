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

    // The mode on at power-up follows the knobs. The other starts from
    // its own settings, and its knobs pick up from there ..........
    if (!_started) {
        _started = true;
        if (_voice) {
            _drum_tune.Init(.5f);
            _drum_tone.Init(0.f);
            _drum_decay.Init(.5f);
            _drum_hardness.Init(.4f);
            _drum_level.Init(.5f);
        } else {
            _voice_pitch.Init(.5f);
            _voice_spread.Init(.2f);
            _voice_swell.Init(.45f);
            _voice_unsteady.Init(.5f);
            _voice_level.Init(.5f);
        }
    }

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

    // The loops, both in either mode. A take only counts; a loop plays
    // whether or not the other one is taking .....................
    bool recording = false;
    for (auto& loop : _loops) {
        if (loop.recording) {
            recording = true;
            loop.tick++;
            if (++loop.idle >= kLoopIdleTicks || loop.tick >= 60000) _end_take(loop, true);
        }
        if (loop.length > 0) {
            auto from = loop.phase;
            auto to = from + _loop_rate;
            _play(loop, from, to);
            if (to >= (float)loop.length) {
                to -= (float)loop.length;
                _play(loop, 0.f, to);
            }
            loop.phase = to;
        }
    }

    // LED flashes on everything played, in either mode, and while
    // recording stays on and blinks off for everything it takes ....
    if (_led_ticks > 0) _led_ticks--;
    auto flash = _led_ticks > 0;
    hw.SetLed(recording ? !flash : flash);
};

// P10 takes and P11 clears, in either mode, the loop of that mode. Tap
// P10, and what you play until the next tap comes round again from the
// first thing you played: drum hits into the drum loop, vowels into the
// voice loop, each where you stood. A vowel still down when the take
// ends is the loop's from then on and keeps singing when the pad comes
// up. A take with nothing played for a while ends itself.
void MoundUI::_on_pad_touch(uint16_t pad) {
    if (pad == kLoopClearPad) {
        _clear(_loop());
        return;
    }
    if (pad == kLoopRecordPad) {
        auto& loop = _loop();
        if (loop.recording) _end_take(loop, false);
        else _begin_take(loop);
        return;
    }
    if (_voice) {
        for (uint8_t i = 0; i < kVoicePadCount; i++) {
            if (kVoicePads[i].pad != pad) continue;
            auto at = _mound.Listener();
            auto& loop = _loops[1];
            auto taken = loop.recording && _record(loop, SING, i, at);
            _start(i, at, false, taken);
            return;
        }
        return;
    }
    // A place on the skin, or a stone: one strike index across both.
    for (uint8_t i = 0; i < kDrumPadCount; i++) {
        auto hit = i < kSkinPadCount ? kSkinPads[i].pad : kStonePads[i - kSkinPadCount].pad;
        if (hit != pad) continue;
        auto at = _mound.Listener();
        auto& loop = _loops[0];
        if (loop.recording) _record(loop, STRIKE, i, at);
        _strike(i, at);
        return;
    }
};

void MoundUI::_on_pad_release(uint16_t pad) {
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].held || _voices[v].pad < 0 || kVoicePads[_voices[v].pad].pad != pad) continue;
        auto& loop = _loops[1];
        if (loop.recording && _voices[v].recorded) _record(loop, REST, _voices[v].pad, _mound.Listener());
        _stop(v);
    }
};

// Gone, and the voices it held with it.
void MoundUI::_clear(Loop& loop) {
    loop.recording = false;
    loop.count = 0;
    loop.length = 0;
    loop.tick = 0;
    loop.phase = 0.f;
    if (&loop != &_loops[1]) return;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (_voices[v].held) _stop(v);
        _voices[v].recorded = false;
    }
};

// A take starts a loop, and a loop already running goes, held vowels
// and all. In VOICE, vowels under your fingers count as pressed at the
// start.
void MoundUI::_begin_take(Loop& loop) {
    if (loop.length > 0) _clear(loop);
    loop.recording = true;
    loop.idle = 0;
    loop.count = 0;
    loop.tick = 0;
    loop.phase = 0.f;
    if (!_voice) return;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        if (!_voices[v].sounding || _voices[v].pad < 0) continue;
        _voices[v].recorded = _record(loop, SING, _voices[v].pad, _voices[v].at);
    }
};

// Nothing played, nothing loops. A vowel taken during the take and
// still down is the loop's now. A take that ended itself is trimmed,
// one ended by a tap is exactly as long as the tap made it.
void MoundUI::_end_take(Loop& loop, bool trim) {
    loop.recording = false;
    loop.length = loop.count > 0 ? _bar(loop, loop.tick, trim) : 0;
    loop.tick = 0;
    loop.phase = 0.f;
    if (&loop != &_loops[1]) return;
    for (auto& v : _voices) {
        if (loop.length > 0 && v.recorded && v.sounding && v.pad >= 0) v.held = true;
        v.recorded = false;
    }
};

// Where in the bar a thing taken now lands: the count so far. Steps
// keep time order. A vowel is taken only if there is room for its end
// as well.
bool MoundUI::_record(Loop& loop, Kind kind, uint8_t index, float at) {
    loop.idle = 0;
    auto room = kind == SING ? kLoopSteps - 1 : kLoopSteps;
    if (loop.count >= room) return false;
    uint16_t tick = loop.tick;
    uint8_t i = loop.count++;
    while (i > 0 && loop.steps[i - 1].tick > tick) {
        loop.steps[i] = loop.steps[i - 1];
        i--;
    }
    loop.steps[i] = { tick, kind, index, at };
    _led_ticks = kLedStrikeTicks;
    return true;
};

// Every step from one tick up to the next, at whatever speed S30 says.
void MoundUI::_play(Loop& loop, const float from, const float to) {
    for (uint8_t i = 0; i < loop.count; i++) {
        auto& step = loop.steps[i];
        auto tick = (float)step.tick;
        if (tick >= from && tick < to) _replay(step);
    }
};

// A loop plays a hit where it was struck. A vowel it starts is its to
// hold. One it started last time round and never ended just goes on
// singing rather than starting again, and it never takes a vowel from
// under your fingers.
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
        if (free) _start(step.index, step.at, true, false);
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
uint16_t MoundUI::_bar(Loop& loop, const uint16_t hold, const bool trim) {
    auto lead = loop.steps[0].tick;
    for (uint8_t i = 0; i < loop.count; i++) loop.steps[i].tick -= lead;
    uint16_t length = hold - lead;
    if (trim && loop.count == 1 && loop.steps[0].kind == STRIKE) return 0;
    if (trim && loop.count > 1) {
        std::array<uint16_t, kLoopSteps> gaps;
        uint8_t count = 0;
        for (uint8_t i = 1; i < loop.count; i++) {
            uint16_t gap = loop.steps[i].tick - loop.steps[i - 1].tick;
            uint8_t j = count++;
            while (j > 0 && gaps[j - 1] > gap) {
                gaps[j] = gaps[j - 1];
                j--;
            }
            gaps[j] = gap;
        }
        uint16_t span = loop.steps[loop.count - 1].tick + gaps[count / 2];
        if (span < length) length = span;
    }
    if (length < kLoopMinTicks) length = kLoopMinTicks;
    for (uint8_t i = 0; i < loop.count; i++) {
        if (loop.steps[i].tick >= length) loop.steps[i].tick = length - 1;
    }
    return length;
};

// A pad takes a silent voice first, then one the loop holds, then the
// older of the ones under your fingers. A voice taken from under a
// finger during a take has its end taken too, since it has ended. The
// engine starts a new syllable on a voice already singing.
void MoundUI::_start(uint8_t index, float at, bool held, bool recorded) {
    uint8_t pick = 0;
    uint8_t best = 3;
    uint16_t oldest = 0;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
        uint8_t rank = !_voices[v].sounding ? 0 : (_voices[v].held ? 1 : 2);
        auto age = (uint16_t)(_presses - _voices[v].age);
        if (rank < best || (rank == best && age > oldest)) {
            pick = v;
            best = rank;
            oldest = age;
        }
    }
    auto& old = _voices[pick];
    if (old.sounding && old.recorded && old.pad >= 0 && _loops[1].recording) {
        _record(_loops[1], REST, old.pad, at);
    }
    _voices[pick] = { (int8_t)index, true, held, recorded, ++_presses, at };
    _mound.Sing(pick, kVoicePads[index].vowel, kVoicePads[index].semitones, at);
    _led_ticks = kLedStrikeTicks;
};

void MoundUI::_stop(uint8_t voice) {
    _mound.Rest(voice);
    _voices[voice].pad = -1;
    _voices[voice].sounding = false;
    _voices[voice].held = false;
    _voices[voice].recorded = false;
};
