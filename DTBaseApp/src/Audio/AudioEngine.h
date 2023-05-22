#pragma once
#include "Core/Core.h"

namespace DT
{
	class SoundGroup
	{
	public:
		SoundGroup();
		~SoundGroup();

		void Play();
		void Pause();

		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetPan(float pan);

		void* GetHandle() const { return m_Group; }
	private:
		void* m_Group = nullptr;
	};

	struct SoundProperties
	{
		float Volume = 1.0f;
		float Pitch = 1.0f;
		float Pan = 0.0f;
	};

	class SoundEffect
	{
	public:
		SoundEffect(const char* filePath, SoundGroup* group = nullptr);
		~SoundEffect();

		std::string GetName() const { return m_AssetPath.filename().string(); }

		void* GetHandle() const { return m_Sound; }
		void* GetGroupHandle() const { return m_SoundGroup; }

		static Ref<SoundEffect> Create(const char* filePath, SoundGroup* group = nullptr) { return CreateRef<SoundEffect>(filePath, group); }
	private:
		void* m_Sound = nullptr;
		void* m_SoundGroup = nullptr;
		std::filesystem::path m_AssetPath;
	};

	class Sound
	{
	public:
		Sound(const char* filePath, SoundGroup* group = nullptr);
		~Sound();

		std::string GetName() const { return m_AssetPath.filename().string(); }

		void Play();
		void Pause();
		void Stop();

		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetPan(float pan);
		void SetFade(uint64 milliseconds, float startVolume, float endVolume);

		float GetVolume() const;
		float GetPitch() const;
		float GetPan() const;
		void GetAudioBuffer(std::vector<float>& buffer) const;
		uint64 GetCursorInPcmFrames() const;

		static Ref<Sound> Create(const char* filePath, SoundGroup* group = nullptr) { return CreateRef<Sound>(filePath, group); }
	private:
		void* m_Sound = nullptr;
		std::filesystem::path m_AssetPath;
	};

	namespace AudioNodes
	{
		class LowPassFilter
		{
		public:
			LowPassFilter(float frequency, uint32 order = 2u);
			~LowPassFilter();

			void UpdateParameters(float frequency, uint32 order = 2u);

			float GetFrequency() const { return m_Frequency; }
			uint32 GetOrder() const { return m_Order; }

			void* GetHandle() const { return m_Node; }
			static Ref<LowPassFilter> Create(float frequency, uint32 order = 2u) { return CreateRef<LowPassFilter>(frequency, order); }
		private:
			void* m_Node = nullptr;
			float m_Frequency;
			uint32 m_Order;
		};

		class HighPassFilter
		{
		public:
			HighPassFilter(float frequency, uint32 order = 2u);
			~HighPassFilter();

			void UpdateParameters(float frequency, uint32 order = 2u);

			float GetFrequency() const { return m_Frequency; }
			uint32 GetOrder() const { return m_Order; }

			void* GetHandle() const { return m_Node; }
			static Ref<HighPassFilter> Create(float frequency, uint32 order = 2u) { return CreateRef<HighPassFilter>(frequency, order); }
		private:
			void* m_Node = nullptr;
			float m_Frequency;
			uint32 m_Order;
		};

		class Delay
		{
		public:
			Delay(float delaySeconds, float decay);
			~Delay();

			void* GetHandle() const { return m_Node; }
			static Ref<Delay> Create(float delaySeconds, float decay) { return CreateRef<Delay>(delaySeconds, decay); }
		private:
			void* m_Node = nullptr;
			float m_Delay;
			float m_Decay; 
		};

		class Reverb
		{
		public:
			Reverb();
			~Reverb();

			void* GetHandle() const { return m_Node; }
			static Ref<Reverb> Create() { return CreateRef<Reverb>(); }
		private:
			void* m_Node = nullptr;
		};
	}

	class Audio
	{
	public:
		static void Init();
		static void Shutdown();

		static void Connect(void* sourceNode, void* destinationNode);
		static void ConnectToEndpoint(void* sourceNode);

		static float PeekOutput();
		static void PlaySoundEffect(const Ref<SoundEffect>& sound, const SoundProperties& properties);

		static void SetMasterVolume(float volume);
	};
}
