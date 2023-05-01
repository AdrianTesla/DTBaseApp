#pragma once
#include "Core/Math.h"
#include "Texture.h"

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
		static void DrawRotatedQuad(const glm::vec2& position, float width, float height, float angle, const glm::vec4& color);
		static void DrawTriangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec4& color);
		static void DrawCircle(const glm::vec2& position, float radius, float thickness, float fade, const glm::vec4& color);
		static void DrawLine(const glm::vec2& startPos, const glm::vec2& endPos, float thickness, const glm::vec4& color);
		static void DrawRect(const glm::vec2& position, float width, float height, float thickness, const glm::vec4& color);
		static void DrawRotatedRect(const glm::vec2& position, float width, float height, float thickness, float angle, const glm::vec4& color);
		static void DrawTexturedQuad(const glm::vec2& position, float width, float height, const Ref<Texture2D>& texture, float tiling, const glm::vec4& color);
		static void DrawRotatedTexQuad(const glm::vec2& position, float width, float height, const Ref<Texture2D>& texture, float tiling,float angle, const glm::vec4& color);
	private:
		static void StartBatch();
		static void NextBatch();
		static void ResetStatistics();
		static void Flush();
	};
}
