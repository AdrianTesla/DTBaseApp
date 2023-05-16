#pragma once
#include "Core/Core.h"

namespace DT
{
	class Sound
	{
	public:
		Sound(const char* filePath);
		~Sound();

		void Play();
		void Pause();
		void Stop();

		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetPan(float pan);

		static Ref<Sound> Create(const char* filePath)
		{
			return CreateRef<Sound>(filePath);
		}
	private:
		int32 m_Index = -1;
		void* m_CopiedActiveSound = nullptr;

		float m_Volume = 1.0f;
		float m_Pitch = 1.0f;
		float m_Pan = 0.0f;
	};

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetMasterVolume(float volume);
	};
}
