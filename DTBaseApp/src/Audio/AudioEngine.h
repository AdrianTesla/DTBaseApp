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

		static Ref<SoundEffect> Create(const char* filePath, SoundGroup* group = nullptr) { return CreateRef<SoundEffect>(filePath, group); }
	private:
		void* m_Sound = nullptr;
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
	}

	class Audio
	{
	public:
		static void Init();
		static void Shutdown();

		static void AttachOutputBus(void* sourceNode, void* destinationNode);
		static void AttachOutputBusToEndpoint(void* sourceNode);

		static void PlaySoundEffect(const Ref<SoundEffect>& sound, const SoundProperties& properties);

		static void SetMasterVolume(float volume);
	};
}
