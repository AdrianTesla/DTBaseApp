#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"
#include "Platform/API_Vulkan/VulkanContext.h"
#include <stb_image.h>

namespace DT
{
	static struct UniformBufferData
	{
		float ScreenWidth;
		float ScreenHeight;
		float AspectRatio;
		float Time;
	} s_UniformBufferData;

	void VulkanLearnLayer::OnAttach()
	{
		m_Shader = Ref<VulkanShader>::Create();

		PipelineSpecification specification{};
		specification.Shader = m_Shader;
		specification.PolygonMode = PolygonMode::Fill;
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
		commandBufferAllocateInfo.commandPool		 = vulkanDevice.GetGraphicsCommandPool();
		commandBufferAllocateInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_GraphicsCommandBuffers.Data()));
	}

	void VulkanLearnLayer::CreateBuffers()
	{
		struct Vertex
		{
			struct
			{
				float x;
				float y;
			} Position;
			struct
			{
				float r;
				float g;
				float b;
			} Color;
			struct
			{
				float u;
				float v;
			} TexCoord;
		};

		const float s = 1.0f / std::sqrtf(2.0f);

		Vertex vertices[4];
		vertices[0].Position = { -s,-s };
		vertices[1].Position = { -s,+s };
		vertices[2].Position = { +s,+s };
		vertices[3].Position = { +s,-s };

		vertices[0].Color = { 1.0f,1.0f,1.0f };
		vertices[1].Color = { 1.0f,1.0f,1.0f };
		vertices[2].Color = { 1.0f,1.0f,1.0f };
		vertices[3].Color = { 1.0f,1.0f,1.0f };

		vertices[0].TexCoord = { 0.0f,0.0f };
		vertices[1].TexCoord = { 0.0f,1.0f };
		vertices[2].TexCoord = { 1.0f,1.0f };
		vertices[3].TexCoord = { 1.0f,0.0f };

		uint32 indices[6] = { 0u,1u,2u, 0u,2u,3u };

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

		// uniform buffer
		{
			VkWriteDescriptorSet writeDescriptorSets[MAX_FRAMES_IN_FLIGHT];
			for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				writeDescriptorSets[i].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[i].pNext            = nullptr;
				writeDescriptorSets[i].dstSet           = m_DescriptorSets[i];
				writeDescriptorSets[i].dstBinding       = 0u;
				writeDescriptorSets[i].dstArrayElement  = 0u;
				writeDescriptorSets[i].descriptorCount  = 1u;
				writeDescriptorSets[i].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[i].pImageInfo       = nullptr;
				writeDescriptorSets[i].pBufferInfo      = &m_UniformBuffers[i]->GetDescriptorBufferInfo();
				writeDescriptorSets[i].pTexelBufferView = nullptr;
			}
			vkUpdateDescriptorSets(device, (uint32)std::size(writeDescriptorSets), writeDescriptorSets, 0u, nullptr);
		}

		// combined image sampler
		{
			VkWriteDescriptorSet writeDescriptorSets[MAX_FRAMES_IN_FLIGHT];
			for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				writeDescriptorSets[i].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[i].pNext            = nullptr;
				writeDescriptorSets[i].dstSet           = m_DescriptorSets[i];
				writeDescriptorSets[i].dstBinding       = 1u;
				writeDescriptorSets[i].dstArrayElement  = 0u;
				writeDescriptorSets[i].descriptorCount  = 1u;
				writeDescriptorSets[i].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[i].pImageInfo       = &m_DescriptorImageInfo;
				writeDescriptorSets[i].pBufferInfo      = nullptr;
				writeDescriptorSets[i].pTexelBufferView = nullptr;
			}
			vkUpdateDescriptorSets(device, (uint32)std::size(writeDescriptorSets), writeDescriptorSets, 0u, nullptr);
		}
	}

	void VulkanLearnLayer::CreateImage()
	{
		int32 width = 0, height = 0;
		uint8* imagePixels = (uint8*)stbi_load("assets/textures/tonyboss_3-16.png", &width, &height, nullptr, STBI_rgb_alpha);
		
		ImageSpecification specification{};
		specification.Width = (uint32)width;
		specification.Height = (uint32)height;
		m_Image = Ref<VulkanDynamicImage>::Create(specification);
		
		Buffer pixels = Buffer(m_Image->GetBuffer(), m_Image->GetSize());
		memcpy(pixels.Data, imagePixels, pixels.Size);

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext        = nullptr;
		imageViewCreateInfo.flags        = 0u;
		imageViewCreateInfo.image        = m_Image->GetVulkanImage();
		imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format       = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel   = 0u;
		imageViewCreateInfo.subresourceRange.levelCount     = 1u;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
		imageViewCreateInfo.subresourceRange.layerCount     = 1u;
		VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_ImageView));

		Vulkan::TransitionImageLayout(m_Image->GetVulkanImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		stbi_image_free(imagePixels);
	}

	void VulkanLearnLayer::CreateSampler()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType					  = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext					  = nullptr;
		samplerCreateInfo.flags					  = 0u;
		samplerCreateInfo.magFilter				  = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter				  = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode			  = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerCreateInfo.addressModeU			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeV			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeW			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.mipLodBias			  = 0.0f;
		samplerCreateInfo.anisotropyEnable		  = VK_FALSE;
		samplerCreateInfo.maxAnisotropy			  = 1.0f;
		samplerCreateInfo.compareEnable			  = VK_FALSE;
		samplerCreateInfo.compareOp				  = VK_COMPARE_OP_LESS;
		samplerCreateInfo.minLod				  = 0.0f;
		samplerCreateInfo.maxLod				  = 0.0f;
		samplerCreateInfo.borderColor			  = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		VK_CALL(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Sampler));

		m_DescriptorImageInfo.sampler = m_Sampler;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
	}

	void VulkanLearnLayer::RecordCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VulkanSwapchain& swapchain = VulkanContext::GetSwapchain();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = 0u;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VkClearValue clearValue = {{{ 0.0f,0.0f,0.0f,1.0f }}};

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext             = nullptr;
		renderPassBeginInfo.renderPass        = swapchain.GetSwapchainRenderPass();
		renderPassBeginInfo.framebuffer       = swapchain.GetActiveFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0u,0u };
		renderPassBeginInfo.renderArea.extent = { (uint32)swapchain.GetWidth(),(uint32)swapchain.GetHeight()};
		renderPassBeginInfo.clearValueCount   = 1u;
		renderPassBeginInfo.pClearValues      = &clearValue;

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (float)swapchain.GetWidth();
		viewport.height   = (float)swapchain.GetHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0u,0u };
		scissor.extent = { (uint32)viewport.width,(uint32)viewport.height };

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
			vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1u, 0u, 0u, 0u);
			vkCmdEndRenderPass(commandBuffer);
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanLearnLayer::ExecuteCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VulkanSwapchain& swapchain = VulkanContext::GetSwapchain();
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkDevice device = vulkanDevice.GetVulkanDevice();
		VkQueue graphicsQueue = vulkanDevice.GetGraphicsQueue();

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
		submitInfo.pSignalSemaphores    = &VulkanContext::GetActiveRenderCompleteSemaphore();
		VK_CALL(vkQueueSubmit(graphicsQueue, 1u, &submitInfo, VulkanContext::GetActivePreviousFrameFence()));
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{

		// Supponiamo di avere un moto su traiettoria arbitraria in cui a(t) è esplicitamente nota.
		// Supponiamo che la velocità iniziare sia v(0) = v0, e supponiamo che dopo un tempo T
		// la velocità scende a v(T) = 0 avendo percorso uno spazio D.
		// Dimostrare che D = integrale da 0 a T di ta(t) dt

		// D = int da 0 a T v(t) dt = 
		// = - int da 0 a T t a(t) dt
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
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}
}