#include "Renderer2D.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RenderPass.h"
#include "Renderer/DX11Buffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Sampler.h"
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

	struct TexQuadVertex
	{
		glm::vec2 Position;
		glm::vec2 TexCoord;
		glm::vec4 Color;
		float Tiling;
		int32 TextureIndex;
	};

	template<typename T, uint32 MaxVertices>
	struct Batch
	{
		Ref<VertexBuffer> VertexBuffer;
		Ref<RenderPass> RenderPass;

		T* VertexBufferBase = nullptr;
		T* VertexBufferPtr = nullptr;

		uint32 VertexCount = 0u;

		constexpr uint32 MaxVerts()
		{
			return MaxVertices;
		}
		
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
			Renderer::BeginRenderPass(RenderPass, false);
			Renderer::Draw(VertexCount);
			Renderer::EndRenderPass();
		}
	};

	struct Renderer2DData
	{
		Batch<QuadVertex, 60'000u> QuadBatch;
		Batch<CircleVertex, 60'000u> CircleBatch;
		Batch<TexQuadVertex, 60'000u> TexQuadBatch;

		Renderer2DStatistics Statistics;

		Ref<Texture2D> Textures[32];
		Ref<Sampler> Sampler;
		uint32 TextureCount = 0u;

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
		quadPipelineSpec.BlendingMode = BlendingMode::Alpha;

		RenderPassSpecification quadPassSpec{};
		quadPassSpec.Pipeline = CreateRef<Pipeline>(quadPipelineSpec);
		s_Data->QuadBatch.RenderPass = CreateRef<RenderPass>(quadPassSpec);
		s_Data->QuadBatch.Init();
		
		PipelineSpecification circlePipelineSpec{};
		circlePipelineSpec.VertexShaderPath = "CircleVS.cso";
		circlePipelineSpec.PixelShaderPath = "CirclePS.cso";
		circlePipelineSpec.BlendingMode = BlendingMode::Alpha;

		RenderPassSpecification circlePassSpec{};
		circlePassSpec.Pipeline = CreateRef<Pipeline>(circlePipelineSpec);
		s_Data->CircleBatch.RenderPass = CreateRef<RenderPass>(circlePassSpec);
		s_Data->CircleBatch.Init();

		PipelineSpecification texQuadPipelineSpec{};
		texQuadPipelineSpec.VertexShaderPath = "TexQuadVS.cso";
		texQuadPipelineSpec.PixelShaderPath = "TexQuadPS.cso";
		texQuadPipelineSpec.BlendingMode = BlendingMode::Alpha;

		RenderPassSpecification texQuadPassSpec{};
		texQuadPassSpec.Pipeline = CreateRef<Pipeline>(texQuadPipelineSpec);
		s_Data->TexQuadBatch.RenderPass = CreateRef<RenderPass>(texQuadPassSpec);
		s_Data->TexQuadBatch.Init();

		s_Data->Sampler = CreateRef<Sampler>(true);

		s_Data->CameraUniformBuffer = CreateRef<UniformBuffer>(sizeof(UBCamera));
	}

	void Renderer2D::Shutdown()
	{
		s_Data->QuadBatch.Shutdown();
		s_Data->CircleBatch.Shutdown();
		s_Data->TexQuadBatch.Shutdown();
		delete s_Data;
	}

	void Renderer2D::BeginScene()
	{
		ResetStatistics();

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
		if (s_Data->QuadBatch.VertexCount >= s_Data->QuadBatch.MaxVerts())
			s_Data->QuadBatch.NextBatch();

		glm::vec2 v0 = { position.x - width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v1 = { position.x + width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v2 = { position.x + width / 2.0f, position.y - height / 2.0f };
		glm::vec2 v3 = { position.x - width / 2.0f, position.y - height / 2.0f };
		DrawTriangle(v0, v1, v2, color);
		DrawTriangle(v0, v2, v3, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float width, float height, float angle, const glm::vec4& color)
	{
		if (s_Data->QuadBatch.VertexCount >= s_Data->QuadBatch.MaxVerts())
			s_Data->QuadBatch.NextBatch();

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

		s_Data->Statistics.PolygonCount++;
	}

	void Renderer2D::DrawCircle(const glm::vec2& position, float radius, float thickness, float fade, const glm::vec4& color)
	{
		if (s_Data->CircleBatch.VertexCount >= s_Data->CircleBatch.MaxVerts())
			s_Data->CircleBatch.NextBatch();

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

		s_Data->Statistics.PolygonCount += 2u;
	}

	void Renderer2D::DrawLine(const glm::vec2& startPos, const glm::vec2& endPos, float thickness, const glm::vec4& color)
	{
		if (s_Data->QuadBatch.VertexCount >= s_Data->QuadBatch.MaxVerts())
			s_Data->QuadBatch.NextBatch();

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

	void Renderer2D::DrawTexturedQuad(const glm::vec2& position, float width, float height, const Ref<Texture2D>& texture, float tiling, const glm::vec4& color)
	{
		if (s_Data->TexQuadBatch.VertexCount >= s_Data->TexQuadBatch.MaxVerts())
			s_Data->TexQuadBatch.NextBatch();

		int32 textureIndex = -1;
		for (uint32 i = 0u; i < s_Data->TextureCount; i++)
		{
			if (s_Data->Textures[i]->Compare(texture))
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == -1)
		{
			if (s_Data->TextureCount >= 32u)
			{
				s_Data->TexQuadBatch.NextBatch();
				s_Data->TextureCount = 0u;
			}
			textureIndex = s_Data->TextureCount;
			s_Data->Textures[s_Data->TextureCount++] = texture;
		}
		
		glm::vec2 v0 = { position.x - width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v1 = { position.x + width / 2.0f, position.y + height / 2.0f };
		glm::vec2 v2 = { position.x + width / 2.0f, position.y - height / 2.0f };
		glm::vec2 v3 = { position.x - width / 2.0f, position.y - height / 2.0f };

		//First Triangle 
		s_Data->TexQuadBatch.VertexBufferPtr->Position = v0;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v1;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v2;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		//Second Triangle 
		s_Data->TexQuadBatch.VertexBufferPtr->Position = v0;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v2;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v3;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexCount += 6u;

		s_Data->Statistics.PolygonCount += 2u;
	}

	void Renderer2D::DrawRotatedTexQuad(const glm::vec2& position, float width, float height, const Ref<Texture2D>& texture, float tiling, float angle, const glm::vec4& color)
	{
		if (s_Data->TexQuadBatch.VertexCount >= s_Data->TexQuadBatch.MaxVerts())
			s_Data->TexQuadBatch.NextBatch();

		int32 textureIndex = -1;
		for (uint32 i = 0u; i < s_Data->TextureCount; i++)
		{
			if (s_Data->Textures[i]->Compare(texture))
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == -1)
		{
			if (s_Data->TextureCount >= 32u)
			{
				s_Data->TexQuadBatch.NextBatch();
				s_Data->TextureCount = 0u;
			}
			textureIndex = s_Data->TextureCount;
			s_Data->Textures[s_Data->TextureCount++] = texture;
		}

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

		//First Triangle 
		s_Data->TexQuadBatch.VertexBufferPtr->Position = v0;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v1;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v2;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		//Second Triangle 
		s_Data->TexQuadBatch.VertexBufferPtr->Position = v0;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v2;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexBufferPtr->Position = v3;
		s_Data->TexQuadBatch.VertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data->TexQuadBatch.VertexBufferPtr->Color = color;
		s_Data->TexQuadBatch.VertexBufferPtr->Tiling = tiling;
		s_Data->TexQuadBatch.VertexBufferPtr->TextureIndex = textureIndex;
		s_Data->TexQuadBatch.VertexBufferPtr++;

		s_Data->TexQuadBatch.VertexCount += 6u;

		s_Data->Statistics.PolygonCount += 2u;
	}

	void Renderer2D::SetTargetFramebuffer(const Ref<Framebuffer>& framebuffer)
	{
		s_Data->QuadBatch.RenderPass->GetSpecification().TargetFramebuffer = framebuffer;
		s_Data->CircleBatch.RenderPass->GetSpecification().TargetFramebuffer = framebuffer;
		s_Data->TexQuadBatch.RenderPass->GetSpecification().TargetFramebuffer = framebuffer;
	}

	void Renderer2D::StartBatch()
	{
		s_Data->QuadBatch.Start();
		s_Data->CircleBatch.Start();
		s_Data->TexQuadBatch.Start();
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::ResetStatistics()
	{
		memset(&s_Data->Statistics, 0, sizeof(s_Data->Statistics));
	}

	void Renderer2D::Flush()
	{
		if (s_Data->QuadBatch.VertexCount > 0u)
		{
			s_Data->QuadBatch.Flush();
			s_Data->Statistics.DrawCalls++;
		}

		if (s_Data->CircleBatch.VertexCount > 0u)
		{
			s_Data->CircleBatch.Flush();
			s_Data->Statistics.DrawCalls++;
		}

		if (s_Data->TexQuadBatch.VertexCount > 0u)
		{
			s_Data->Sampler->Bind(0u);

			for (uint32 i = 0u; i < s_Data->TextureCount; i++)
				s_Data->Textures[i]->Bind(i);

			s_Data->TexQuadBatch.Flush();
			s_Data->Statistics.DrawCalls++;
		}

	}

	Renderer2DStatistics& Renderer2D::GetStatistics()
	{
		return s_Data->Statistics;
	}
}
