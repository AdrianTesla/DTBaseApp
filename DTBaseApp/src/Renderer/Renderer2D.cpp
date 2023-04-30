#include "Renderer2D.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Renderer.h"
#include "Core/Application.h"

namespace DT
{
	struct UBCamera
	{
		glm::mat4 ProjectionMatrix;
	};

	struct QuadVertex
	{
		glm::vec2 Position;
		glm::vec4 Color;
	};

	struct CircleVertex
	{
		glm::vec2 Position;
		glm::vec2 LocalPos;
		glm::vec4 Color;
		float Thickness;
		float Fade;
	};

	template<typename T, uint32 MaxVertices>
	struct Batch
	{
		Ref<VertexBuffer> VertexBuffer;
		Ref<Pipeline> Pipeline;

		T* VertexBufferBase = nullptr;
		T* VertexBufferPtr = nullptr;

		uint32 VertexCount = 0u;
		
		void Init()
		{
			VertexBuffer = DT::CreateRef<DT::VertexBuffer>(MaxVertices * sizeof(T));
			VertexBufferBase = new T[MaxVertices];
		}

		void Shutdown()
		{
			delete[] VertexBufferBase;
		}

		void Start()
		{
			VertexBufferPtr = VertexBufferBase;
			VertexCount = 0u;
		}

		void NextBatch()
		{
			Flush();
			Start();
		}

		void Flush()
		{
			VertexBuffer->SetData(VertexBufferBase, sizeof(T) * VertexCount);
			VertexBuffer->Bind(sizeof(T));
			Pipeline->Bind();
		}
	};

	struct Renderer2DData
	{
		Batch<QuadVertex, 60'000u> QuadBatch;
		Batch<CircleVertex, 60'000u> CircleBatch;

		Ref<UniformBuffer> CameraUniformBuffer;

		UBCamera Camera;
	};

	static Renderer2DData* s_Data = nullptr;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DData();

		PipelineSpecification quadPipelineSpec{};
		quadPipelineSpec.VertexShaderPath = "QuadVS.cso";
		quadPipelineSpec.PixelShaderPath = "QuadPS.cso";
		quadPipelineSpec.BlendingEnabled = true;
		s_Data->QuadBatch.Pipeline = CreateRef<Pipeline>(quadPipelineSpec);
		s_Data->QuadBatch.Init();
		
		PipelineSpecification circlePipelineSpec{};
		circlePipelineSpec.VertexShaderPath = "CircleVS.cso";
		circlePipelineSpec.PixelShaderPath = "CirclePS.cso";
		circlePipelineSpec.BlendingEnabled = true;
		s_Data->CircleBatch.Pipeline = CreateRef<Pipeline>(circlePipelineSpec);
		s_Data->CircleBatch.Init();

		s_Data->CameraUniformBuffer = CreateRef<UniformBuffer>(sizeof(UBCamera));
	}

	void Renderer2D::Shutdown()
	{
		s_Data->QuadBatch.Shutdown();
		s_Data->CircleBatch.Shutdown();
		delete s_Data;
	}

	void Renderer2D::BeginScene()
	{
		float aspect = Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight();
		s_Data->Camera.ProjectionMatrix = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
		s_Data->CameraUniformBuffer->SetData(&s_Data->Camera, sizeof(UBCamera));
		s_Data->CameraUniformBuffer->BindVS(0u);
		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, float width, float height, const glm::vec4& color)
	{
		glm::vec2 v0 = { position.x - width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v1 = { position.x + width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v2 = { position.x + width / 2.0f, position.y - height / 2.0f };
		glm::vec2 v3 = { position.x - width / 2.0f, position.y - height / 2.0f };
		DrawTriangle(v0, v1, v2, color);
		DrawTriangle(v0, v2, v3, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float width, float height, float angle, const glm::vec4& color)
	{
		float cosAngle = std::cos(angle);
		float sinAngle = std::sin(angle);
		float halfWidth = 0.5f * width;
		float halfHeight = 0.5f * height;

		glm::vec2 v0 = { -halfWidth * cosAngle - halfHeight * sinAngle, -halfWidth * sinAngle + halfHeight * cosAngle };
		glm::vec2 v1 = { halfWidth * cosAngle - halfHeight * sinAngle, halfWidth * sinAngle + halfHeight * cosAngle };
		glm::vec2 v2 = -v0 + position;
		glm::vec2 v3 = -v1 + position;

		v0 += position;
		v1 += position;

		DrawTriangle(v0, v1, v2, color);
		DrawTriangle(v0, v2, v3, color);
	}

	void Renderer2D::DrawTriangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec4& color)
	{
		s_Data->QuadBatch.VertexBufferPtr->Position = v0;
		s_Data->QuadBatch.VertexBufferPtr->Color = color;
		s_Data->QuadBatch.VertexBufferPtr++;

		s_Data->QuadBatch.VertexBufferPtr->Position = v1;
		s_Data->QuadBatch.VertexBufferPtr->Color = color;
		s_Data->QuadBatch.VertexBufferPtr++;

		s_Data->QuadBatch.VertexBufferPtr->Position = v2;
		s_Data->QuadBatch.VertexBufferPtr->Color = color;
		s_Data->QuadBatch.VertexBufferPtr++;

		s_Data->QuadBatch.VertexCount += 3u;
	}

	void Renderer2D::DrawCircle(const glm::vec2& position, float radius, float thickness, float fade, const glm::vec4& color)
	{
		glm::vec2 v0 = { position.x - radius, position.y + radius };
		glm::vec2 v1 = { position.x + radius, position.y + radius };
		glm::vec2 v2 = { position.x + radius, position.y - radius };
		glm::vec2 v3 = { position.x - radius, position.y - radius };

		//First Triangle
		s_Data->CircleBatch.VertexBufferPtr->Position = v0;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = {-1.0f,1.0f};
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		s_Data->CircleBatch.VertexBufferPtr->Position = v1;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = { 1.0f,1.0f };
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		s_Data->CircleBatch.VertexBufferPtr->Position = v2;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = { 1.0f, -1.0f };
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		//second triangle
		s_Data->CircleBatch.VertexBufferPtr->Position = v0;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = { -1.0f,1.0f };
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		s_Data->CircleBatch.VertexBufferPtr->Position = v2;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = { 1.0f,-1.0f };
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		s_Data->CircleBatch.VertexBufferPtr->Position = v3;
		s_Data->CircleBatch.VertexBufferPtr->LocalPos = { -1.0f,-1.0f };
		s_Data->CircleBatch.VertexBufferPtr->Thickness = thickness;
		s_Data->CircleBatch.VertexBufferPtr->Fade = fade;
		s_Data->CircleBatch.VertexBufferPtr->Color = color;
		s_Data->CircleBatch.VertexBufferPtr++;

		s_Data->CircleBatch.VertexCount += 6u;
	}

	void Renderer2D::DrawLine(const glm::vec2& startPos, const glm::vec2& endPos, float thickness, const glm::vec4& color)
	{
		glm::vec2 direction = glm::normalize(endPos - startPos) * (thickness * 0.5f);
		direction = { -direction.y, direction.x };

		glm::vec2 v0 = startPos + direction;
		glm::vec2 v1 = endPos + direction;
		glm::vec2 v2 = endPos - direction;
		glm::vec2 v3 = startPos - direction;

		DrawTriangle(v0, v1, v2, color);
		DrawTriangle(v0, v2, v3, color);
	}

	void Renderer2D::DrawRect(const glm::vec2& position, float width, float height, float thickness, const glm::vec4& color)
	{
		glm::vec2 v0 = { position.x - width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v1 = { position.x + width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v2 = { position.x + width / 2.0f, position.y - height / 2.0f };
		glm::vec2 v3 = { position.x - width / 2.0f, position.y - height / 2.0f };

		glm::vec2 o0 = glm::normalize(v1 - v0) * (thickness * 0.5f);
		glm::vec2 o1 = glm::normalize(v2 - v1) * (thickness * 0.5f);
		glm::vec2 o2 = glm::normalize(v3 - v2) * (thickness * 0.5f);
		glm::vec2 o3 = glm::normalize(v0 - v3) * (thickness * 0.5f);

		DrawLine(v0 - o0, v1 + o0, thickness, color);
		DrawLine(v1 - o1, v2 + o1, thickness, color);
		DrawLine(v2 - o2, v3 + o2, thickness, color);
		DrawLine(v3 - o3, v0 + o3, thickness, color);
	}

	void Renderer2D::DrawRotatedRect(const glm::vec2& position, float width, float height, float thickness, float angle, const glm::vec4& color)
	{
		float cosAngle = std::cos(angle);
		float sinAngle = std::sin(angle);
		float halfWidth = 0.5f * width;
		float halfHeight = 0.5f * height;

		glm::vec2 v0 = { -halfWidth * cosAngle - halfHeight * sinAngle, -halfWidth * sinAngle + halfHeight * cosAngle };
		glm::vec2 v1 = { halfWidth * cosAngle - halfHeight * sinAngle, halfWidth * sinAngle + halfHeight * cosAngle };
		glm::vec2 v2 = - v0 + position;
		glm::vec2 v3 = - v1 + position;

		v0 += position;
		v1 += position;

		glm::vec2 o0 = glm::normalize(v1 - v0) * (thickness * 0.5f);
		glm::vec2 o1 = glm::normalize(v2 - v1) * (thickness * 0.5f);
		glm::vec2 o2 = glm::normalize(v3 - v2) * (thickness * 0.5f);
		glm::vec2 o3 = glm::normalize(v0 - v3) * (thickness * 0.5f);

		DrawLine(v0 - o0, v1 + o0, thickness, color);
		DrawLine(v1 - o1, v2 + o1, thickness, color);
		DrawLine(v2 - o2, v3 + o2, thickness, color);
		DrawLine(v3 - o3, v0 + o3, thickness, color);
	}

	void Renderer2D::StartBatch()
	{
		s_Data->QuadBatch.Start();
		s_Data->CircleBatch.Start();
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
		s_Data->QuadBatch.Flush();
		Renderer::Draw(s_Data->QuadBatch.VertexCount);

		s_Data->CircleBatch.Flush();
		Renderer::Draw(s_Data->CircleBatch.VertexCount);
	}

}
