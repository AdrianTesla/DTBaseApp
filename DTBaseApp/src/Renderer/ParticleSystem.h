#pragma once
#include <vector>
#include "Core/Math.h"
#include "Core/Core.h"

namespace DT
{
	struct ParticleProperties
	{
		glm::vec2 Position = { 0.0f, 0.0f };
		glm::vec2 Velocity = { 0.0f, 0.0f };
		float VelocityVariation = 1.0f;
		float Lifetime = 1.0f;
		float RotationVariation = 1.0f;
		float StartSize = 0.01f;
		float EndSize = 0.000f;
		float StartEmission = 1.0f;
		float EndEmission = 1.0f;

		glm::vec4 StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 EndColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class ParticleSystem
	{
	public:
		ParticleSystem();
		void EmitParticle(const ParticleProperties& properties);
		void OnUpdate(float dt);
		void OnRender(float fade);

		struct Particle
		{
			glm::vec2 Position;
			glm::vec2 Velocity;
			glm::vec4 CurrentColor;
			glm::vec4 StartColor;
			glm::vec4 EndColor;
			float CurrentLife;        
			float Lifetime;
			float Angle;
			float AngularVelocity;
			float CurrentSize;
			float StartSize;
			float EndSize;
			float StartEmission;
			float CurrentEmission;
			float EndEmission;
		};
	private:
		std::vector<Particle> m_Particles;
		uint32 m_AliveParticles;
	};
}