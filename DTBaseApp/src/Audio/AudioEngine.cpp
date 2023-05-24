#include "AudioEngine.h"
#include "Core/Log.h"
#include "Core/Application.h"

#include <miniaudio.h>
#include "extras/nodes/ma_reverb_node/ma_reverb_node.h"

#define MA_CALL(call) { ma_result res = (call); if (res != MA_SUCCESS) { LOG_ERROR(ma_result_description(res)); __debugbreak(); } }

namespace DT
{

	struct AudioEngineData
	{
		ma_device Device;
		ma_engine Engine;
		ma_node_graph* NodeGraph = nullptr;
		uint32 Channels;
		uint32 SampleRate;
		float LastOutput = 0.0f;
	};
	
	static AudioEngineData* s_AudioData = nullptr;

	static void AudioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		// when the audio card is hungry 
		ma_engine* engine = (ma_engine*)pDevice->pUserData;
		uint64 framesRead;
		MA_CALL(ma_engine_read_pcm_frames(engine, pOutput, frameCount, &framesRead));
		
		uint32 sampleCount = frameCount * s_AudioData->Channels;
		float sum = 0.0f;
		for (uint32 i = 0; i < sampleCount; i++)
			sum += *((float*)pOutput + i);

		Application::Get().SubmitToMainThread([sum, sampleCount]()
		{
			s_AudioData->LastOutput = sum / sampleCount;
		});
	}

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
		: m_AssetPath(filePath), m_SoundGroup(nullptr)
	{
		m_Sound = new ma_sound;

		if (group)
			m_SoundGroup = group->GetHandle();

		MA_CALL(ma_sound_init_from_file(&s_AudioData->Engine,
			filePath,
			MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			(ma_sound_group*)m_SoundGroup, nullptr,
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

	void Sound::SetCursorInPcmFrames(uint64 frameIndex)
	{
		MA_CALL(ma_sound_seek_to_pcm_frame((ma_sound*)m_Sound, frameIndex));
	}

	void Sound::SetCursorInSeconds(float seconds)
	{
		SetCursorInPcmFrames((uint64)(seconds * s_AudioData->SampleRate));
	}

	void Sound::SetLooping(bool looping)
	{
		ma_sound_set_looping((ma_sound*)m_Sound, (ma_bool32)looping);
	}

	uint64 Sound::GetCursorInPcmFrames() const
	{
		uint64 cursor;
		uint64 frameCount;
		MA_CALL(ma_sound_get_cursor_in_pcm_frames((ma_sound*)m_Sound, &cursor));
		MA_CALL(ma_sound_get_length_in_pcm_frames((ma_sound*)m_Sound, &frameCount));
		return cursor % frameCount;
	}

	float Sound::GetCursorInSeconds() const
	{
		float seconds;
		MA_CALL(ma_sound_get_cursor_in_seconds((ma_sound*)m_Sound, &seconds));
		return seconds;
	}

	bool Sound::IsLooping() const
	{
		return (bool)ma_sound_is_looping((ma_sound*)m_Sound);
	}

	bool Sound::IsPlaying() const
	{
		return ma_sound_is_playing((ma_sound*)m_Sound);
	}

	bool Sound::IsAtEnd() const
	{
		return ma_sound_at_end((ma_sound*)m_Sound);
	}

	float Sound::GetVolume() const
	{
		return ma_sound_get_volume((const ma_sound*)m_Sound);
	}

	float Sound::GetPitch() const
	{
		return ma_sound_get_pitch((const ma_sound*)m_Sound);
	}

	float Sound::GetPan() const
	{
		return ma_sound_get_pan((const ma_sound*)m_Sound);
	}

	float Sound::GetLengthInSeconds() const
	{
		float seconds;
		MA_CALL(ma_sound_get_length_in_seconds((ma_sound*)m_Sound, &seconds));
		return seconds;
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
		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.dataCallback = AudioCallback;
		deviceConfig.pUserData = &s_AudioData->Engine;
		MA_CALL(ma_device_init(nullptr, &deviceConfig, &s_AudioData->Device));

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pDevice = &s_AudioData->Device;
		MA_CALL(ma_engine_init(&engineConfig, &s_AudioData->Engine));
		
		s_AudioData->Channels = ma_engine_get_channels(&s_AudioData->Engine);
		s_AudioData->NodeGraph = ma_engine_get_node_graph(&s_AudioData->Engine);
		s_AudioData->SampleRate = ma_engine_get_sample_rate(&s_AudioData->Engine);
	}

	void Audio::Shutdown()
	{
		ma_engine_uninit(&s_AudioData->Engine);

		delete s_AudioData;
		s_AudioData = nullptr;
	}

	void Audio::Connect(void* sourceNode, void* destinationNode)
	{
		MA_CALL(ma_node_attach_output_bus((ma_node*)sourceNode, 0u, (ma_node*)destinationNode, 0u));
	}

	void Audio::ConnectToEndpoint(void* sourceNode)
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

	float Audio::PeekOutput()
	{
		return s_AudioData->LastOutput;
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
				s_AudioData->Channels, 
				s_AudioData->SampleRate, 
				(double)frequency, order);

			MA_CALL(ma_lpf_node_init(s_AudioData->NodeGraph, &lpfNodeConfig, nullptr, (ma_lpf_node*)m_Node));
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
				s_AudioData->Channels, 
				s_AudioData->SampleRate,
				m_Frequency, m_Order);

			MA_CALL(ma_lpf_node_reinit(&lpfConfig, (ma_lpf_node*)m_Node));
		}

		HighPassFilter::HighPassFilter(float frequency, uint32 order)
			: m_Frequency(frequency), m_Order(order)
		{
			m_Node = new ma_hpf_node;
			ma_hpf_node_config hpfNodeConfig = ma_hpf_node_config_init(
				s_AudioData->Channels,
				s_AudioData->SampleRate,
				(double)frequency, order);

			MA_CALL(ma_hpf_node_init(s_AudioData->NodeGraph, &hpfNodeConfig, nullptr, (ma_hpf_node*)m_Node));
		}

		HighPassFilter::~HighPassFilter()
		{
			ma_hpf_node_uninit((ma_hpf_node*)m_Node, nullptr);
			delete m_Node;
		}

		void HighPassFilter::UpdateParameters(float frequency, uint32 order)
		{
			m_Frequency = frequency;
			m_Order = order;

			ma_hpf_config hpfConfig = ma_hpf_config_init(ma_format_f32,
				s_AudioData->Channels,
				s_AudioData->SampleRate,
				m_Frequency, m_Order);

			MA_CALL(ma_hpf_node_reinit(&hpfConfig, (ma_hpf_node*)m_Node));
		}

		Delay::Delay(float delaySeconds, float decay)
			: m_Delay(delaySeconds), m_Decay(decay)
		{
			m_Node = new ma_delay_node;
			uint32 delayInFrames = (uint32)(s_AudioData->SampleRate * delaySeconds);
			ma_delay_node_config delayNodeConfig = ma_delay_node_config_init(s_AudioData->Channels, s_AudioData->SampleRate, delayInFrames, decay);
			MA_CALL(ma_delay_node_init(s_AudioData->NodeGraph, &delayNodeConfig, nullptr, (ma_delay_node*)m_Node));
		}

		Delay::~Delay()
		{
			ma_delay_node_uninit((ma_delay_node*)m_Node, nullptr);
			delete m_Node;
		}

		Reverb::Reverb()
		{
			m_Node = new ma_reverb_node;
			ma_reverb_node_config reverbNodeConfig = ma_reverb_node_config_init(s_AudioData->Channels, s_AudioData->SampleRate);
			MA_CALL(ma_reverb_node_init(s_AudioData->NodeGraph, &reverbNodeConfig, nullptr, (ma_reverb_node*)m_Node));
		}

		Reverb::~Reverb()
		{
			ma_reverb_node_uninit((ma_reverb_node*)m_Node, nullptr);
			delete m_Node;
		}

		void Reverb::SetRoomSize(float roomSize)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_room_size(&node->reverb, roomSize);
		}

		void Reverb::SetDryWet(float dryWet)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_dry(&node->reverb, 1.0f - dryWet);
			verblib_set_wet(&node->reverb, dryWet);
		}

		void Reverb::SetDamping(float damping)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_damping(&node->reverb, damping);
		}

		void Reverb::SetStereoWidth(float width)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_width(&node->reverb, width);
		}

		void Reverb::SetInputStereoWidth(float inputWidth)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_input_width(&node->reverb, inputWidth);
		}

		void Reverb::SetMode(float mode)
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			verblib_set_mode(&node->reverb, mode);
		}

		float Reverb::GetDryWet() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_wet(&node->reverb);
		}

		float Reverb::GetRoomSize() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_room_size(&node->reverb);
		}

		float Reverb::GetDamping() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_damping(&node->reverb);
		}

		float Reverb::GetStereoWidth() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_width(&node->reverb);
		}

		float Reverb::GetInputStereoWidth() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_input_width(&node->reverb);
		}

		float Reverb::GetMode() const
		{
			ma_reverb_node* node = (ma_reverb_node*)m_Node;
			return verblib_get_mode(&node->reverb);
		}
	}
}