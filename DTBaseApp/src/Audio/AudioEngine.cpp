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
		ma_sound Sounds[1000];
		uint32 SoundCount = 0u;
	};
	
	static AudioEngineData* s_AudioData = nullptr;

	static void OnReleaseSound(void* pUserData, ma_sound* pSound)
	{
		//This will be executed at the end of the sound
		Application::Get().SubmitToMainThread([pSound, pUserData]()
		{
			ma_sound_uninit(pSound);
			delete pSound;
			void** pCopiedActiveSound = (void**)pUserData;
			*pCopiedActiveSound = nullptr;
		});
	}

	Sound::Sound(const char* filePath)
	{
		m_Index = s_AudioData->SoundCount++;

		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr,
			&s_AudioData->Sounds[m_Index]));
	}

	Sound::~Sound()
	{
		ma_sound_uninit(&s_AudioData->Sounds[m_Index]);
	}

	void Sound::Play()
	{
		if(!m_CopiedActiveSound)
		{
			ma_sound* copiedSound = new ma_sound;
			m_CopiedActiveSound = copiedSound;
			MA_CALL(ma_sound_init_copy(&s_AudioData->Engine, &s_AudioData->Sounds[m_Index], 0u, nullptr, copiedSound));
			ma_sound_set_volume(copiedSound, m_Volume);
			ma_sound_set_pitch(copiedSound, m_Pitch);
			ma_sound_set_pan(copiedSound, m_Pan);
			MA_CALL(ma_sound_set_end_callback(copiedSound, OnReleaseSound, &m_CopiedActiveSound));
		}
		MA_CALL(ma_sound_start((ma_sound*)m_CopiedActiveSound));
	}

	void Sound::Pause()
	{
		if(m_CopiedActiveSound)
			MA_CALL(ma_sound_stop((ma_sound*)m_CopiedActiveSound));
	}

	void Sound::Stop()
	{
		if (m_CopiedActiveSound)
		{
			MA_CALL(ma_sound_stop((ma_sound*)m_CopiedActiveSound));
			MA_CALL(ma_sound_seek_to_pcm_frame((ma_sound*)m_CopiedActiveSound, 0u));
		}
	}

	void Sound::SetVolume(float volume)
	{
		m_Volume = volume;

		if(m_CopiedActiveSound)
			ma_sound_set_volume((ma_sound*)m_CopiedActiveSound, m_Volume);
	}

	void Sound::SetPitch(float pitch)
	{
		m_Pitch = pitch;

		if (m_CopiedActiveSound)
			ma_sound_set_pitch((ma_sound*)m_CopiedActiveSound, m_Pitch);
	}

	void Sound::SetPan(float pan)
	{
		m_Pan = pan;

		if (m_CopiedActiveSound)
			ma_sound_set_pan((ma_sound*)m_CopiedActiveSound, m_Pan);
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

	void AudioEngine::SetMasterVolume(float volume)
	{
		MA_CALL(ma_engine_set_volume(&s_AudioData->Engine, volume));
	}

}