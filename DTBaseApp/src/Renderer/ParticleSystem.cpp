#include "ParticleSystem.h"
#include "glm/gtc/random.hpp"
#include "Renderer2D.h"

namespace DT
{
	ParticleSystem::ParticleSystem()
	{
		m_Particles.resize(1u);
		m_AliveParticles = 0u;
	}

	void ParticleSystem::EmitParticle(const ParticleProperties& properties)
	{
		if (m_AliveParticles >= m_Particles.size())
			m_Particles.resize(m_Particles.size() * 2u);

		Particle& particle = m_Particles[m_AliveParticles++];
		particle.CurrentLife = 0.0f;
		particle.Lifetime = properties.Lifetime;
		particle.Position = properties.Position;
		particle.Velocity = properties.Velocity;

		if(properties.VelocityVariation > 0.0f)
			particle.Velocity += glm::diskRand(properties.VelocityVariation);

		particle.StartColor = properties.StartColor;
		particle.EndColor = properties.EndColor;
		particle.CurrentColor = particle.StartColor;

		particle.StartEmission = properties.StartEmission;
		particle.EndEmission = properties.EndEmission;
		particle.CurrentEmission = properties.StartEmission;

		particle.StartSize = properties.StartSize;
		particle.EndSize = properties.EndSize;
		particle.CurrentSize = particle.StartSize;

		particle.AngularVelocity = properties.RotationVariation * (rand() / 65'536.0f - 0.5f) * 2.0f;
		particle.Angle = 3.1415f * (rand() / (float)RAND_MAX * 2.0f - 1.0f);
	}

	void ParticleSystem::OnUpdate(float dt)
	{
		for (uint32 i = 0u; i < m_AliveParticles; i++)
		{
			Particle& particle = m_Particles[i];

			if (particle.CurrentLife >= particle.Lifetime)
			{
				m_AliveParticles--;
				std::swap(particle, m_Particles[m_AliveParticles]);
				continue;
			}

			//Update current particle parameters
			particle.Position += particle.Velocity * dt;
			particle.Angle += particle.AngularVelocity * dt;

			float fraction = particle.CurrentLife / particle.Lifetime;

			particle.CurrentColor = glm::mix(particle.StartColor, particle.EndColor, fraction);
			particle.CurrentSize = glm::mix(particle.StartSize, particle.EndSize, fraction);
			particle.CurrentEmission = glm::mix(particle.StartEmission, particle.EndEmission, fraction);
			particle.CurrentLife += dt;
		}
	}

	void ParticleSystem::OnRender(float fade)
	{
		for (uint32 i = 0u; i < m_AliveParticles; i++)
		{
			Particle& particle = m_Particles[i];
			glm::vec4 color = particle.CurrentColor;
			color.x *= particle.CurrentEmission;
			color.y *= particle.CurrentEmission;
			color.z *= particle.CurrentEmission;
			Renderer2D::DrawRotatedQuad(particle.Position, particle.CurrentSize, particle.CurrentSize, particle.Angle, color);
			//Renderer2D::DrawCircle(particle.Position, particle.CurrentSize, 1.0f, fade, color);
		}
	}
}