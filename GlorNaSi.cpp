#include "daisy_seed.h"
#include "touch/touch.h"
#include "mound/mound.h"
#include "ui/mound_ui.h"
#include "config.h"
#include "log.h"

using namespace daisy;
using namespace synthux;

DaisySeed hw;

Touch touch;
Mound engine;
MoundUI ui(touch, engine);

#if DEBUG
// Callback cost in CPU cycles, cleared after every report ...........
static volatile uint32_t cycles_max = 0;
static volatile uint32_t cycles_sum = 0;
static volatile uint32_t cycles_count = 0;
#endif

void AudioCallback(
	AudioHandle::InputBuffer in, 
	AudioHandle::OutputBuffer out, 
	size_t size) {
	#if DEBUG
	auto start = DWT->CYCCNT;
	#endif
	engine.Process(out, size);
	#if DEBUG
	auto cycles = DWT->CYCCNT - start;
	if (cycles > cycles_max) cycles_max = cycles;
	cycles_sum += cycles;
	cycles_count++;
	#endif
};

int main(void) {
	hw.Init(true); // 480 MHz
	hw.SetAudioBlockSize(kAudioBlockSize);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	#if DEBUG
	HW::hw().setHW(&hw);
	HW::hw().startLog();
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	auto budget = hw.AudioBlockSize() * (System::GetSysClkFreq() / (uint32_t)hw.AudioSampleRate());
	uint16_t report_ticks = 0;
	#endif

	touch.Init(hw);
	engine.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	ui.Init(hw);

	hw.StartAudio(AudioCallback);

	while(1) {
		ui.Process(hw);

		#if DEBUG
		// Once a second: callback size, worst and mean cycles, budget .
		if (++report_ticks >= 250) {
			report_ticks = 0;
			__disable_irq();
			auto max = cycles_max;
			auto sum = cycles_sum;
			auto count = cycles_count;
			cycles_max = cycles_sum = cycles_count = 0;
			__enable_irq();
			HW::hw().print("%s gate %d size %u max %lu mean %lu budget %lu peak %lu%%",
				touch.switches().A() == Switch3::POS_UP ? "VOICE" : "DRUM",
				touch.pads().HasTouch(), hw.AudioBlockSize(),
				max, count ? sum / count : 0, budget, budget ? max * 100 / budget : 0);
		}
		#endif

		System::Delay(4);
	}
};
