#include "Renderer2D.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Renderer.h"

namespace DT
{
	struct QuadVertex
	{
		glm::vec2 Position;
		glm::vec4 Color;
	};

	struct Renderer2DData
	{
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Pipeline> QuadPipeline;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32 QuadVertexCount = 0u;
	};

	static Renderer2DData* s_Data = nullptr;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DData();
		s_Data->QuadPipeline = CreateRef<Pipeline>();
		s_Data->QuadVertexBuffer = CreateRef<VertexBuffer>(1'000'000u);

		s_Data->QuadVertexBufferBase = new QuadVertex[125'000];
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Data->QuadVertexBufferBase;
		delete s_Data;
	}

	void Renderer2D::BeginScene()
	{
		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::DrawQuad(const glm::vec2& center, float width, float height, const glm::vec4& color)
	{
		glm::vec2 v0 = { center.x - width / 2.0f , center.y + height / 2.0f };
		glm::vec2 v1 = { center.x + width / 2.0f , center.y + height / 2.0f };
		glm::vec2 v2 = { center.x + width / 2.0f , center.y - height / 2.0f };
		glm::vec2 v3 = { center.x - width / 2.0f , center.y - height / 2.0f };
		DrawTriangle(v0, v1, v2, color);
		DrawTriangle(v0, v2, v3, color);
	}

	void Renderer2D::DrawTriangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec4& color)
	{
		s_Data->QuadVertexBufferPtr->Position = v0;
		s_Data->QuadVertexBufferPtr->Color = color;
		s_Data->QuadVertexBufferPtr++;

		s_Data->QuadVertexBufferPtr->Position = v1;
		s_Data->QuadVertexBufferPtr->Color = color;
		s_Data->QuadVertexBufferPtr++;

		s_Data->QuadVertexBufferPtr->Position = v2;
		s_Data->QuadVertexBufferPtr->Color = color;
		s_Data->QuadVertexBufferPtr++;

		s_Data->QuadVertexCount += 3u;
	}

	void Renderer2D::StartBatch()
	{
		s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;
		s_Data->QuadVertexCount = 0u;
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::ResetStatistics()
	{
	}

	void Renderer2D::Flush()
	{
		s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, s_Data->QuadVertexCount * sizeof(QuadVertex));
		s_Data->QuadVertexBuffer->Bind(sizeof(QuadVertex));
		s_Data->QuadPipeline->Bind();
		Renderer::Draw(s_Data->QuadVertexCount);
	}

}
