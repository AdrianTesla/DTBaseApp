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
		std::vector<Ref<Sound>> AliveSounds;

	};
	
	static AudioEngineData* s_AudioData = nullptr;

	Sound::Sound()
	{
		m_Sound = new ma_sound;
	}

	Sound::Sound(const char* filePath)
	{
		m_Sound = new ma_sound;
		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr,
			(ma_sound*)m_Sound));
	}

	Sound::~Sound()
	{
		ma_sound_uninit((ma_sound*)m_Sound);
		delete m_Sound;
	}

	Ref<Sound> Sound::Copy(const Ref<Sound>& sound)
	{
		Ref<Sound> copySound = CreateRef<Sound>();
		MA_CALL(ma_sound_init_copy(&s_AudioData->Engine, (const ma_sound*)sound->m_Sound, 0u, nullptr, (ma_sound*)copySound->m_Sound));
		return copySound;
	}

	void AudioEngine::Init()
	{
		s_AudioData = new AudioEngineData;
		
		ma_engine_config engineConfig = ma_engine_config_init();
		MA_CALL(ma_engine_init(&engineConfig, &s_AudioData->Engine));
	}

	void AudioEngine::Shutdown()
	{
		ma_engine_uninit(&s_AudioData->Engine);

		delete s_AudioData;
		s_AudioData = nullptr;
	}

	Ref<Sound> AudioEngine::LoadFromFile(const char* filePath)
	{
		return CreateRef<Sound>(filePath);
	}

	void AudioEngine::Play(const Ref<Sound>& sound)
	{
		Ref<Sound> copySound = Sound::Copy(sound);
		s_AudioData->AliveSounds.emplace_back(copySound);
		MA_CALL(ma_sound_start((ma_sound*)copySound->m_Sound));
	}

	void AudioEngine::SetMasterVolume(float volume)
	{
		MA_CALL(ma_engine_set_volume(&s_AudioData->Engine, volume));
	}

}