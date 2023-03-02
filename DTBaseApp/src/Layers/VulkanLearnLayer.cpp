#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"
#include "Platform/API_Vulkan/VulkanContext.h"
#include "Platform/API_Vulkan/VulkanRenderer.h"
#include <stb_image.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace DT
{
	static struct UniformBufferData
	{
		glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 ViewMatrix = glm::mat4(1.0f);
		float ScreenWidth;
		float ScreenHeight;
		float AspectRatio;
		float Time;
	} s_UniformBufferData{};

	static glm::mat4 s_Cube0Transform = glm::mat4(1.0f);
	static glm::mat4 s_Cube1Transform = glm::mat4(1.0f);

	void VulkanLearnLayer::OnAttach()
	{
		PipelineSpecification specification{};
		specification.Shader = Ref<VulkanShader>::Create();
		specification.PolygonMode = PolygonMode::Fill;
		specification.Topology = PrimitiveTopology::TriangleList;
		specification.Culling = FaceCulling::Back;
		m_Pipeline = Ref<VulkanPipeline>::Create(specification);

		CreateCommandBuffers();
		CreateBuffers();
		CreateImage();
		CreateSampler();
		
		CreateDescriptorPools();
		CreateDescriptorSets();
	}

	void VulkanLearnLayer::CreateCommandBuffers()
	{
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext				 = nullptr;
		commandBufferAllocateInfo.commandPool		 = vulkanDevice.GetCommandPool(QueueType::Graphics);
		commandBufferAllocateInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_GraphicsCommandBuffers.Data()));
	}

	void VulkanLearnLayer::CreateBuffers()
	{
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Color;
			glm::vec2 TexCoord;
		};

		const float s = 0.5f;

		Vertex vertices[8];
		vertices[0].Position = { -s,-s,-s };
		vertices[1].Position = { -s,-s,+s };
		vertices[2].Position = { +s,-s,+s };
		vertices[3].Position = { +s,-s,-s };
		vertices[4].Position = { -s,+s,-s };
		vertices[5].Position = { -s,+s,+s };
		vertices[6].Position = { +s,+s,+s };
		vertices[7].Position = { +s,+s,-s };

		for (uint8 i = 0u; i < 8u; i++) {
			float r = 1.0f;
			float g = 1.0f;
			float b = 1.0f;
			vertices[i].Color = { r,g,b };
		}

		vertices[0].TexCoord = { 0.0f,0.0f };
		vertices[1].TexCoord = { 0.0f,1.0f };
		vertices[2].TexCoord = { 1.0f,1.0f };
		vertices[3].TexCoord = { 1.0f,0.0f };
		vertices[4].TexCoord = { 0.0f,0.0f };
		vertices[5].TexCoord = { 0.0f,1.0f };
		vertices[6].TexCoord = { 1.0f,1.0f };
		vertices[7].TexCoord = { 1.0f,0.0f };

		constexpr uint32 indices[] = { 
			0u,2u,1u,
			2u,0u,3u,
			0u,5u,4u,
			5u,0u,1u,
			5u,1u,6u,
			6u,1u,2u,
			2u,7u,6u,
			7u,2u,3u,
			4u,7u,3u,
			3u,0u,4u,
			7u,4u,5u,
			7u,5u,6u
		};

		constexpr uint32 line_indices[] = { 
			0u,1u,
			1u,2u,
			2u,3u,
			3u,0u,
			0u,4u,
			1u,5u,
			2u,6u,
			3u,7u,
			4u,5u,
			5u,6u,
			6u,7u,
			7u,4u
		};

		m_VertexBuffer = Ref<VulkanVertexBuffer>::Create(vertices, sizeof(vertices));
		m_IndexBuffer = Ref<VulkanIndexBuffer>::Create(indices, sizeof(indices));

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			m_UniformBuffers[i] = Ref<VulkanUniformBuffer>::Create(sizeof(UniformBufferData));
	}

	void VulkanLearnLayer::UpdateUniformBuffers()
	{
		float width = (float)Application::Get().GetWindow().GetWidth();
		float height = (float)Application::Get().GetWindow().GetHeight();

		s_UniformBufferData.ScreenWidth  = width;
		s_UniformBufferData.ScreenHeight = height;
		s_UniformBufferData.AspectRatio  = width / height;
		s_UniformBufferData.ProjectionMatrix = glm::perspective(45.0f, s_UniformBufferData.AspectRatio, 0.1f, 1000.0f);
		s_UniformBufferData.ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
		s_UniformBufferData.Time = (float)Application::Get().GetTime();
		
		m_UniformBuffers[Renderer::CurrentFrame()]->SetData(&s_UniformBufferData, sizeof(s_UniformBufferData));
	}

	void VulkanLearnLayer::CreateDescriptorPools()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkDescriptorPoolSize descriptorPoolSizes[2];
		descriptorPoolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

		descriptorPoolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext         = nullptr;
		descriptorPoolCreateInfo.flags         = 0u;
		descriptorPoolCreateInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
		descriptorPoolCreateInfo.poolSizeCount = (uint32)std::size(descriptorPoolSizes);
		descriptorPoolCreateInfo.pPoolSizes    = descriptorPoolSizes;
		VK_CALL(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool));
	}

	void VulkanLearnLayer::CreateDescriptorSets()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkDescriptorSetLayout descriptorSetLayout = m_Pipeline->GetDescriptorSetLayout();
		VkDescriptorSetLayout descriptorSetLayouts[MAX_FRAMES_IN_FLIGHT];
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			descriptorSetLayouts[i] = descriptorSetLayout;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext              = nullptr;
		descriptorSetAllocateInfo.descriptorPool     = m_DescriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		descriptorSetAllocateInfo.pSetLayouts        = descriptorSetLayouts;
		VK_CALL(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, m_DescriptorSets.Data()));

		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		writeDescriptorSets.reserve(2u * MAX_FRAMES_IN_FLIGHT);

		// in-flight uniform buffers
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();

			writeDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.pNext            = nullptr;
			writeDescriptorSet.dstSet           = m_DescriptorSets[i];
			writeDescriptorSet.dstBinding       = 0u;
			writeDescriptorSet.dstArrayElement  = 0u;
			writeDescriptorSet.descriptorCount  = 1u;
			writeDescriptorSet.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pImageInfo       = nullptr;
			writeDescriptorSet.pBufferInfo      = &m_UniformBuffers[i]->GetDescriptorBufferInfo();
			writeDescriptorSet.pTexelBufferView = nullptr;
		}

		// combined image sampler
		VkDescriptorImageInfo descriptorImageInfo{};
		descriptorImageInfo.sampler		= m_Sampler;
		descriptorImageInfo.imageView	= m_Texture->GetVulkanImageView();
		descriptorImageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();

			writeDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.pNext            = nullptr;
			writeDescriptorSet.dstSet           = m_DescriptorSets[i];
			writeDescriptorSet.dstBinding       = 1u;
			writeDescriptorSet.dstArrayElement  = 0u;
			writeDescriptorSet.descriptorCount  = 1u;
			writeDescriptorSet.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pImageInfo       = &descriptorImageInfo;
			writeDescriptorSet.pBufferInfo      = nullptr;
			writeDescriptorSet.pTexelBufferView = nullptr;
		}

		vkUpdateDescriptorSets(device, (uint32)writeDescriptorSets.size(), writeDescriptorSets.data(), 0u, nullptr);
	}

	void VulkanLearnLayer::CreateImage()
	{
		TextureSpecification specification{};
		specification.AssetPath = "assets/textures/M_FloorTiles1_Inst_0_BaseColor.png";
		m_Texture = Ref<VulkanTexture2D>::Create(specification);
	}

	void VulkanLearnLayer::CreateSampler()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType					  = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext					  = nullptr;
		samplerCreateInfo.flags					  = 0u;
		samplerCreateInfo.magFilter				  = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter				  = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode			  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU			  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV			  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW			  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias			  = 0.0f;
		samplerCreateInfo.anisotropyEnable		  = VK_FALSE;
		samplerCreateInfo.maxAnisotropy			  = 1.0f;
		samplerCreateInfo.compareEnable			  = VK_FALSE;
		samplerCreateInfo.compareOp				  = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod				  = 0.0f;
		samplerCreateInfo.maxLod				  = 0.0f;
		samplerCreateInfo.borderColor			  = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		VK_CALL(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Sampler));
	}

	void VulkanLearnLayer::RecordCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VulkanSwapchain& swapchain = VulkanRenderer::GetSwapchain();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = 0u;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VkClearValue clearValues[2];
		clearValues[0].color              = {{ 0.0f,0.0f,0.0f,1.0f }};
		clearValues[1].depthStencil.depth = 1.0f;

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext             = nullptr;
		renderPassBeginInfo.renderPass        = swapchain.GetSwapchainRenderPass();
		renderPassBeginInfo.framebuffer       = swapchain.GetActiveFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0u,0u };
		renderPassBeginInfo.renderArea.extent = { (uint32)swapchain.GetWidth(),(uint32)swapchain.GetHeight()};
		renderPassBeginInfo.clearValueCount   = (uint32)std::size(clearValues);
		renderPassBeginInfo.pClearValues      = clearValues;

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = (float)swapchain.GetHeight();
		viewport.width    = (float)swapchain.GetWidth();
		viewport.height   = -(float)swapchain.GetHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0u,0u };
		scissor.extent = { swapchain.GetWidth(),swapchain.GetHeight() };

		VkDeviceSize offsets = 0u;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetVulkanPipeline());
			vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &m_VertexBuffer->GetVulkanBuffer(), &offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetVulkanBuffer(), 0u, VK_INDEX_TYPE_UINT32);
			vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
			vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0u, 1u, &m_DescriptorSets[Renderer::CurrentFrame()], 0u, nullptr);
			vkCmdPushConstants(commandBuffer, m_Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(glm::mat4), &s_Cube0Transform);
			vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1u, 0u, 0u, 0u);
			vkCmdPushConstants(commandBuffer, m_Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(glm::mat4), &s_Cube1Transform);
			vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1u, 0u, 0u, 0u);
			vkCmdEndRenderPass(commandBuffer);
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanLearnLayer::ExecuteCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VulkanSwapchain& swapchain = VulkanRenderer::GetSwapchain();
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkDevice device = vulkanDevice.GetVulkanDevice();
		VkQueue graphicsQueue = vulkanDevice.GetQueue(QueueType::Graphics);

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.waitSemaphoreCount   = 1u;
		submitInfo.pWaitSemaphores      = &swapchain.GetImageAvailableSemaphore(Renderer::CurrentFrame());
		submitInfo.pWaitDstStageMask    = &waitStage;
		submitInfo.commandBufferCount   = 1u;
		submitInfo.pCommandBuffers      = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1u;
		submitInfo.pSignalSemaphores    = &VulkanRenderer::GetActiveRenderCompleteSemaphore();
		VK_CALL(vkQueueSubmit(graphicsQueue, 1u, &submitInfo, VulkanRenderer::GetActivePreviousFrameFence()));
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
		float t = Application::Get().GetTime();

		float factor = (0.5f + 0.5f * std::sin(t)) * 2.0f;

		float xRotation = 0.2f * t;
		float yRotation = 0.3f * t;
		float zRotation = 0.4f * t;

		s_Cube0Transform = glm::mat4(1.0f);
		s_Cube0Transform = glm::rotate(s_Cube0Transform, +xRotation, glm::vec3(1.0f, 0.0f, 0.0f));
		s_Cube0Transform = glm::rotate(s_Cube0Transform, +yRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		s_Cube0Transform = glm::rotate(s_Cube0Transform, +zRotation, glm::vec3(0.0f, 0.0f, 1.0f));

		s_Cube1Transform = glm::mat4(1.0f);
		s_Cube1Transform = glm::rotate(s_Cube1Transform, -xRotation, glm::vec3(1.0f, 0.0f, 0.0f));
		s_Cube1Transform = glm::rotate(s_Cube1Transform, -yRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		s_Cube1Transform = glm::rotate(s_Cube1Transform, -zRotation, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void VulkanLearnLayer::OnRender()
	{
		UpdateUniformBuffers();
		RecordCommandBuffer(m_GraphicsCommandBuffers[Renderer::CurrentFrame()]);
		ExecuteCommandBuffer(m_GraphicsCommandBuffers[Renderer::CurrentFrame()]);
	}

	void VulkanLearnLayer::OnEvent(Event& event)
	{
	}

	void VulkanLearnLayer::OnDetach()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VK_CALL(vkDeviceWaitIdle(device));

		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

		m_Sampler = VK_NULL_HANDLE;		
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}