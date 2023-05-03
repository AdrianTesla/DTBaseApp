#include "ParticleSystem.h"
#include "glm/gtc/random.hpp"
#include "Renderer2D.h"

namespace DT
{
	ParticleSystem::ParticleSystem()
	{
		m_Particles.resize(1000u);
		m_AliveParticles = 0u;
	}

	void ParticleSystem::EmitParticle(const ParticleProperties& properties)
	{
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
		particle.AngularVelocity = properties.RotationVariation * (rand() / 65'536.0f - 0.5f) * 2.0f;
		particle.Angle = 0.0f;
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
			particle.CurrentLife += dt;
		}
	}

	void ParticleSystem::OnRender(float fade)
	{
		for (uint32 i = 0u; i < m_AliveParticles; i++)
		{
			Particle& particle = m_Particles[i];
			Renderer2D::DrawRotatedQuad(particle.Position, 0.05f, 0.01f, particle.Angle, particle.CurrentColor);
			//Renderer2D::DrawCircle(particle.Position, 0.03f, 1.0f, fade, 1.0f - particle.CurrentColor);
		}
	}
}