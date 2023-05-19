#pragma once
#include "Core/Core.h"

namespace DT
{
	struct SoundProperties
	{
		float Volume = 1.0f;
		float Pitch = 1.0f;
		float Pan = 0.0f;
	};

	class SoundEffect
	{
	public:
		SoundEffect(const char* filePath);
		~SoundEffect();

		std::string GetName() const { return m_AssetPath.filename().string(); }

		void* GetHandle() const { return m_Sound; }

		static Ref<SoundEffect> Create(const char* filePath) { return CreateRef<SoundEffect>(filePath); }
	private:
		void* m_Sound = nullptr;
		std::filesystem::path m_AssetPath;
	};

	class Sound
	{
	public:
		Sound(const char* filePath);
		~Sound();

		std::string GetName() const { return m_AssetPath.filename().string(); }

		void Play();
		void Pause();
		void Stop();

		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetPan(float pan);

		static Ref<Sound> Create(const char* filePath) { return CreateRef<Sound>(filePath); }
	private:
		void* m_Sound = nullptr;
		std::filesystem::path m_AssetPath;
	};

	class Audio
	{
	public:
		static void Init();
		static void Shutdown();

		static void PlaySoundEffect(const Ref<SoundEffect>& sound, const SoundProperties& properties);

		static void SetMasterVolume(float volume);
	};
}
