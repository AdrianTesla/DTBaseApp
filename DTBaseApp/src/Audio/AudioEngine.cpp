#include "AudioEngine.h"
#include "Core/Log.h"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#define MA_CALL(call) { ma_result res = (call); if (res != MA_SUCCESS) { LOG_ERROR(ma_result_description(res)); __debugbreak(); } }

namespace DT
{
	struct AudioEngineData
	{
		ma_engine Engine;
	};
	
	static AudioEngineData* s_AudioData = nullptr;

	void AudioEngine::Init()
	{
		s_AudioData = new AudioEngineData;
		
		ma_engine_config engineConfig = ma_engine_config_init();
		MA_CALL(ma_engine_init(&engineConfig, &s_AudioData->Engine));

		ma_engine_play_sound(&s_AudioData->Engine, "assets/sounds/Infected Mushroom & Ganja White Nights - Kill to Feel.mp3", nullptr);
	}

	void AudioEngine::Shutdown()
	{
		ma_engine_uninit(&s_AudioData->Engine);

		delete s_AudioData;
		s_AudioData = nullptr;
	}
}