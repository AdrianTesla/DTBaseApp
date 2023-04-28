#pragma once
#include "Core/Math.h"

namespace DT
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene();
		static void EndScene();

		static void DrawQuad(const glm::vec2& position, float width, float height, const glm::vec4& color);
		static void DrawTriangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec4& color);
		static void DrawCircle(const glm::vec2& position, float radius, float thickness, float fade, const glm::vec4& color);
	private:
		static void StartBatch();
		static void NextBatch();
		static void ResetStatistics();
		static void Flush();
	};
}
