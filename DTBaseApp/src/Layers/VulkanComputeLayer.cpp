#include "VulkanComputeLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/API_Vulkan/VulkanContext.h"
#include "Platform/API_Vulkan/VulkanRenderer.h"

#include "Renderer/Renderer.h"

namespace DT
{
	struct Vertex
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec3 Color;
	};

	static constexpr uint32 s_VertexCount = 100u;

	void VulkanComputeLayer::OnAttach()
	{
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkDevice device = vulkanDevice.GetVulkanDevice();
		VkCommandPool graphicsPool = vulkanDevice.GetCommandPool(QueueType::Graphics);

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType			     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext			     = nullptr;
		commandBufferAllocateInfo.commandPool	     = graphicsPool;
		commandBufferAllocateInfo.level			     = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_CommandBuffers.Data()));

		m_VertexBuffer = Ref<VulkanVertexBuffer>::Create(s_VertexCount * sizeof(Vertex));
		Vertex vertices[s_VertexCount];
		for (auto& v : vertices)
		{
			auto rng = []()
			{
				return 2.0f * rand() / (float)RAND_MAX - 1.0f;
			};
			v.Position = { 0.9f * rng(),0.9f * rng() };
			v.Velocity = glm::diskRand(0.1f);
			v.Color = { 1.0f,1.0f,0.0f };
		}
		m_VertexBuffer->SetData(vertices, sizeof(vertices));

		// create pipeline
		ShaderSpecification shaderSpecification;
		shaderSpecification.VertexSpirvPath = "assets/shaders/spirv/particle_vert.spv";
		shaderSpecification.FragmentSpirvPath = "assets/shaders/spirv/particle_frag.spv";

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Shader = Ref<VulkanShader>::Create(shaderSpecification);
		pipelineSpecification.PolygonMode = PolygonMode::Fill;
		pipelineSpecification.Topology = PrimitiveTopology::PointList;
		m_Pipeline = Ref<VulkanPipeline>::Create(pipelineSpecification);
	}

	void VulkanComputeLayer::OnUpdate(float dt)
	{
		static float t = 0.0f; t += dt;

		auto rng = []()
		{
			return 2.0f * rand() / (float)RAND_MAX - 1.0f;
		};

		Vertex oldVertices[s_VertexCount];
		m_VertexBuffer->ReadData(oldVertices, sizeof(oldVertices));

		Vertex vertices[s_VertexCount];
		for (uint32 i = 0u; i < s_VertexCount; i++) {
			vertices[i].Position = oldVertices[i].Position + oldVertices[i].Velocity * dt;
			vertices[i].Velocity = oldVertices[i].Velocity;
			vertices[i].Color = oldVertices[i].Color;

			// RIGHT SIDE
			if (vertices[i].Position.x >= 1.0f)
				vertices[i].Velocity.x = -vertices[i].Velocity.x;

			// UP SIDE
			if (vertices[i].Position.y >= 1.0f)
				vertices[i].Velocity.y = -vertices[i].Velocity.y;

			// LEFT SIDE
			if (vertices[i].Position.x <= -1.0f)
				vertices[i].Velocity.x = -vertices[i].Velocity.x;

			// BOTTOM SIDE
			if (vertices[i].Position.y <= -1.0f)
				vertices[i].Velocity.y = -vertices[i].Velocity.y;
		}
		
		m_VertexBuffer->SetData(vertices, sizeof(vertices));
	}

	void VulkanComputeLayer::OnRender()
	{
		// record command buffer for current frame
		{
			VkCommandBuffer commandBuffer = m_CommandBuffers[Renderer::CurrentFrame()];

			VulkanSwapchain& swapchain = VulkanRenderer::GetSwapchain();

			VkClearValue clearValues[2];
			clearValues[0].color = { 0.2f,0.0f,0.35f,1.0f };
			clearValues[1].depthStencil = { 1.0f,0u };

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType			  = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext			  = nullptr;
			renderPassBeginInfo.renderPass		  = swapchain.GetSwapchainRenderPass();
			renderPassBeginInfo.framebuffer		  = swapchain.GetActiveFramebuffer();
			renderPassBeginInfo.renderArea.offset = { 0u,0u };
			renderPassBeginInfo.renderArea.extent = { swapchain.GetWidth(),swapchain.GetHeight() };
			renderPassBeginInfo.clearValueCount	  = (uint32)std::size(clearValues);
			renderPassBeginInfo.pClearValues	  = clearValues;

			VkViewport viewport;
			viewport.x		  = 0.0f;
			viewport.y		  = (float)swapchain.GetHeight();
			viewport.width	  = (float)swapchain.GetWidth();
			viewport.height	  = -(float)swapchain.GetHeight();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor;
			scissor.offset = { 0u,0u };
			scissor.extent.width = swapchain.GetWidth();
			scissor.extent.height = swapchain.GetHeight();

			VkDeviceSize offsets = 0u;
			VkCommandBufferBeginInfo commandBufferBeginInfo;
			commandBufferBeginInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.pNext			= nullptr;
			commandBufferBeginInfo.flags			= 0u;
			commandBufferBeginInfo.pInheritanceInfo	= nullptr;
			VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetVulkanPipeline());
				vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
				vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
				vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &m_VertexBuffer->GetVulkanBuffer(), &offsets);
				vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdDraw(commandBuffer, s_VertexCount, 1u, 0u, 0u);
				vkCmdEndRenderPass(commandBuffer);
			}
			VK_CALL(vkEndCommandBuffer(commandBuffer));
		}

		// submit command buffer for current frame
		{
			VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
			VkQueue graphicsQueue = vulkanDevice.GetQueue(QueueType::Graphics);

			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submitInfo{};
			submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext				= nullptr;
			submitInfo.waitSemaphoreCount	= 1u;
			submitInfo.pWaitSemaphores		= &VulkanRenderer::GetSwapchain().GetImageAvailableSemaphore(Renderer::CurrentFrame());
			submitInfo.pWaitDstStageMask	= &waitStage;
			submitInfo.commandBufferCount	= 1u;
			submitInfo.pCommandBuffers		= &m_CommandBuffers[Renderer::CurrentFrame()];
			submitInfo.signalSemaphoreCount	= 1u;
			submitInfo.pSignalSemaphores	= &VulkanRenderer::GetActiveRenderCompleteSemaphore();
			VK_CALL(vkQueueSubmit(graphicsQueue, 1u, &submitInfo, VulkanRenderer::GetActivePreviousFrameFence()));
		}
	}

	void VulkanComputeLayer::OnEvent(Event& event)
	{
	}

	void VulkanComputeLayer::OnDetach()
	{
		VK_CALL(vkDeviceWaitIdle(VulkanContext::GetCurrentVulkanDevice()));
	}
}