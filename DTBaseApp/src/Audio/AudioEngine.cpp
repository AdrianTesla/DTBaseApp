#include "AudioEngine.h"
#include "Core/Log.h"
#include "Core/Application.h"
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

	static void OnSoundEnd(void* pUserData, ma_sound* pSound)
	{
		//This will be executed at the end of the sound
		Application::Get().SubmitToMainThread([pSound]()
		{
			ma_sound_uninit(pSound);
			delete pSound;
		});
	}

	Sound::Sound(const char* filePath)
		: m_AssetPath(filePath)
	{
		m_Sound = new ma_sound;

		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr,
			sound));
	}

	Sound::~Sound()
	{
		ma_sound_uninit((ma_sound*)m_Sound);
		delete m_Sound;
	}

	void Sound::Play()
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_start(sound));
	}

	void Sound::Pause()
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_stop(sound));
	}

	void Sound::Stop()
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_stop(sound));
		MA_CALL(ma_sound_seek_to_pcm_frame(sound, 0u));
	}

	void Sound::SetVolume(float volume)
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		ma_sound_set_volume(sound, volume);
	}

	void Sound::SetPitch(float pitch)
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		ma_sound_set_pitch(sound, pitch);
	}

	void Sound::SetPan(float pan)
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		ma_sound_set_pan(sound, pan);
	}

	SoundEffect::SoundEffect(const char* filePath)
		: m_AssetPath(filePath)
	{
		m_Sound = new ma_sound;

		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr,
			sound));
	}

	SoundEffect::~SoundEffect()
	{
		ma_sound_uninit((ma_sound*)m_Sound);
		delete m_Sound;
	}

	void Audio::Init()
	{
		s_AudioData = new AudioEngineData;
		MA_CALL(ma_engine_init(nullptr, &s_AudioData->Engine));
	}

	void Audio::Shutdown()
	{
		ma_engine_uninit(&s_AudioData->Engine);

		delete s_AudioData;
		s_AudioData = nullptr;
	}

	void Audio::PlaySoundEffect(const Ref<SoundEffect>& sound, const SoundProperties& properties)
	{
		ma_sound* copiedSound = new ma_sound;
		MA_CALL(ma_sound_init_copy(&s_AudioData->Engine, (const ma_sound*)sound->GetHandle(), 0u, nullptr, copiedSound));
		MA_CALL(ma_sound_set_end_callback(copiedSound, OnSoundEnd, nullptr));

		ma_sound_set_volume(copiedSound, properties.Volume);
		ma_sound_set_pitch(copiedSound, properties.Pitch);
		ma_sound_set_pan(copiedSound, properties.Pan);

		MA_CALL(ma_sound_start(copiedSound));
	}

	void Audio::SetMasterVolume(float volume)
	{
		MA_CALL(ma_engine_set_volume(&s_AudioData->Engine, volume));
	}
}