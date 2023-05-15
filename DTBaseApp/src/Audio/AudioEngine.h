#pragma once
#include "Core/Core.h"

namespace DT
{
	class Sound
	{
	public:
		Sound();
		~Sound();
		static Ref<Sound> Copy(const Ref<Sound>& sound);
		Sound(const char* filePath);
	private:
	private:
		void* m_Sound = nullptr;
		friend class AudioEngine;
	};

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<Sound> LoadFromFile(const char* filePath);
		static void Play(const Ref<Sound>& sound);

		static void SetMasterVolume(float volume);
	};
}
