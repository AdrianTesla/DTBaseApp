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

	SoundGroup::SoundGroup()
	{
		m_Group = new ma_sound_group;

		ma_sound_group* group = (ma_sound_group*)m_Group;
		MA_CALL(ma_sound_group_init(&s_AudioData->Engine, 0u, nullptr, group));
	}

	SoundGroup::~SoundGroup()
	{
		ma_sound_group_uninit((ma_sound_group*)m_Group);
		delete m_Group;
	}

	void SoundGroup::Play()
	{
		MA_CALL(ma_sound_group_start((ma_sound_group*)m_Group));
	}

	void SoundGroup::Pause()
	{
		MA_CALL(ma_sound_group_stop((ma_sound_group*)m_Group));
	}

	void SoundGroup::SetVolume(float volume)
	{
		ma_sound_group_set_volume((ma_sound_group*)m_Group, volume);
	}

	void SoundGroup::SetPitch(float pitch)
	{
		ma_sound_group_set_pitch((ma_sound_group*)m_Group, pitch);
	}

	void SoundGroup::SetPan(float pan)
	{
		ma_sound_group_set_pan((ma_sound_group*)m_Group, pan);
	}

	Sound::Sound(const char* filePath, SoundGroup* group)
		: m_AssetPath(filePath)
	{
		m_Sound = new ma_sound;

		ma_sound_group* soundGroup = nullptr;
		if (group)
			soundGroup = (ma_sound_group*)group->GetHandle();

		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			soundGroup, nullptr,
			(ma_sound*)m_Sound));
	}

	Sound::~Sound()
	{
		ma_sound_uninit((ma_sound*)m_Sound);
		delete m_Sound;
	}

	void Sound::Play()
	{
		MA_CALL(ma_sound_start((ma_sound*)m_Sound));
	}

	void Sound::Pause()
	{
		MA_CALL(ma_sound_stop((ma_sound*)m_Sound));
	}

	void Sound::Stop()
	{
		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_stop(sound));
		MA_CALL(ma_sound_seek_to_pcm_frame(sound, 0u));
	}

	void Sound::SetVolume(float volume)
	{
		ma_sound_set_volume((ma_sound*)m_Sound, volume);
	}

	void Sound::SetPitch(float pitch)
	{
		ma_sound_set_pitch((ma_sound*)m_Sound, pitch);
	}

	void Sound::SetPan(float pan)
	{
		ma_sound_set_pan((ma_sound*)m_Sound, pan);
	}

	void Sound::SetFade(uint64 milliseconds, float startVolume, float endVolume)
	{
		ma_sound_set_fade_in_milliseconds((ma_sound*)m_Sound, startVolume, endVolume, milliseconds);
	}

	float Sound::GetVolume()
	{
		return ma_sound_get_volume((const ma_sound*)m_Sound);
	}

	float Sound::GetPitch()
	{
		return ma_sound_get_pitch((const ma_sound*)m_Sound);
	}

	float Sound::GetPan()
	{
		return ma_sound_get_pan((const ma_sound*)m_Sound);
	}

	void Sound::GetAudioBuffer(std::vector<float>& buffer)
	{
		uint64 frameCount;
		MA_CALL(ma_sound_get_length_in_pcm_frames((ma_sound*)m_Sound, &frameCount));
		buffer.resize(frameCount * ma_engine_get_channels(&s_AudioData->Engine));
		uint64 framesRead;
		Stop();
		MA_CALL(ma_data_source_read_pcm_frames(ma_sound_get_data_source((const ma_sound*)m_Sound), buffer.data(), frameCount, &framesRead));
		Stop();
	}

	SoundEffect::SoundEffect(const char* filePath, SoundGroup* group)
		: m_AssetPath(filePath), m_SoundGroup(nullptr)
	{
		m_Sound = new ma_sound;

		if (group)
			m_SoundGroup = group->GetHandle();

		ma_sound* sound = (ma_sound*)m_Sound;
		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine, filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			(ma_sound_group*)m_SoundGroup, 
			nullptr, sound));
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

	void Audio::AttachOutputBus(void* sourceNode, void* destinationNode)
	{
		MA_CALL(ma_node_attach_output_bus((ma_node*)sourceNode, 0u, (ma_node*)destinationNode, 0u));
	}

	void Audio::AttachOutputBusToEndpoint(void* sourceNode)
	{
		MA_CALL(ma_node_attach_output_bus((ma_node*)sourceNode, 0u, ma_engine_get_endpoint(&s_AudioData->Engine), 0u));
	}

	void Audio::PlaySoundEffect(const Ref<SoundEffect>& sound, const SoundProperties& properties)
	{
		ma_sound* copiedSound = new ma_sound;
		MA_CALL(ma_sound_init_copy(&s_AudioData->Engine, 
			(const ma_sound*)sound->GetHandle(), 0u, 
			(ma_sound_group*)sound->GetGroupHandle(), copiedSound));

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

	namespace AudioNodes
	{
		LowPassFilter::LowPassFilter(float frequency, uint32 order)
			: m_Frequency(frequency), m_Order(order)
		{
			m_Node = new ma_lpf_node;
			ma_lpf_node_config lpfNodeConfig = ma_lpf_node_config_init(
				ma_engine_get_channels(&s_AudioData->Engine), 
				ma_engine_get_sample_rate(&s_AudioData->Engine), 
				(double)frequency, 
				order);

			MA_CALL(ma_lpf_node_init(ma_engine_get_node_graph(&s_AudioData->Engine), &lpfNodeConfig, nullptr, (ma_lpf_node*)m_Node));
		}

		LowPassFilter::~LowPassFilter()
		{
			ma_lpf_node_uninit((ma_lpf_node*)m_Node, nullptr);
			delete m_Node;
		}

		void LowPassFilter::UpdateParameters(float frequency, uint32 order)
		{
			m_Frequency = frequency;
			m_Order = order;

			ma_lpf_config lpfConfig = ma_lpf_config_init(ma_format_f32,
				ma_engine_get_channels(&s_AudioData->Engine), 
				ma_engine_get_sample_rate(&s_AudioData->Engine),
				m_Frequency, m_Order);

			MA_CALL(ma_lpf_node_reinit(&lpfConfig, (ma_lpf_node*)m_Node));
		}

		Delay::Delay(float delaySeconds, float decay)
			: m_Delay(delaySeconds), m_Decay(decay)
		{
			m_Node = new ma_delay_node;
			uint32 delayInFrames = (uint32)(ma_engine_get_sample_rate(&s_AudioData->Engine) * delaySeconds);
			ma_delay_node_config delayNodeConfig = ma_delay_node_config_init(ma_engine_get_channels(&s_AudioData->Engine), ma_engine_get_sample_rate(&s_AudioData->Engine), delayInFrames, decay);
			MA_CALL(ma_delay_node_init(ma_engine_get_node_graph(&s_AudioData->Engine), &delayNodeConfig, nullptr, (ma_delay_node*)m_Node));
		}

		Delay::~Delay()
		{
			ma_delay_node_uninit((ma_delay_node*)m_Node, nullptr);
			delete m_Node;
		}
	}
}