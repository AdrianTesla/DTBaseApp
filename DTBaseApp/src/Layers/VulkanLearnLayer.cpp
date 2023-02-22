#include "VulkanLearnLayer.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Platform/PlatformUtils.h"
#include "Platform/API_Vulkan/VulkanContext.h"

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
		vertices[1].Color = { 0.0f,0.0f,1.0f };
		vertices[2].Color = { 0.0f,0.0f,0.0f };
		vertices[3].Color = { 0.0f,1.0f,1.0f };

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
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext				  = nullptr;
		imageCreateInfo.flags				  = 0u;
		imageCreateInfo.imageType			  = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format				  = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent				  = { 16u,16u,1u };
		imageCreateInfo.mipLevels			  = 1u;
		imageCreateInfo.arrayLayers			  = 1u;
		imageCreateInfo.samples				  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationInfo allocationInfo;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_ImageAllocation, &allocationInfo));

		VkImageSubresource imageSubresource{};
		imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresource.mipLevel   = 0u;
		imageSubresource.arrayLayer = 0u;
		VkSubresourceLayout subresourceLayout;
		vkGetImageSubresourceLayout(device, m_Image, &imageSubresource, &subresourceLayout);

		m_ImageData = (Pixel*)allocationInfo.pMappedData;
		m_ImageRowPitch = subresourceLayout.rowPitch;

		LOG_INFO("Image size: {} bytes", subresourceLayout.size);
		LOG_INFO("Image offset: {} bytes", subresourceLayout.offset);
		LOG_INFO("Image row pitch: {} bytes", subresourceLayout.rowPitch);
		LOG_INFO("Image depth pitch: {} bytes", subresourceLayout.depthPitch);
		LOG_INFO("Image array pitch: {} bytes", subresourceLayout.arrayPitch);

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext        = nullptr;
		imageViewCreateInfo.flags        = 0u;
		imageViewCreateInfo.image        = m_Image;
		imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format       = imageCreateInfo.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel   = 0u;
		imageViewCreateInfo.subresourceRange.levelCount     = 1u;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
		imageViewCreateInfo.subresourceRange.layerCount     = 1u;
		VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_ImageView));

		Vulkan::TransitionImageLayout(m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
		samplerCreateInfo.addressModeU			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW			  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias			  = 0.0f;
		samplerCreateInfo.anisotropyEnable		  = VK_FALSE;
		samplerCreateInfo.maxAnisotropy			  = 1.0f;
		samplerCreateInfo.compareEnable			  = VK_FALSE;
		samplerCreateInfo.compareOp				  = VK_COMPARE_OP_LESS;
		samplerCreateInfo.minLod				  = 0.0f;
		samplerCreateInfo.maxLod				  = 0.0f;
		samplerCreateInfo.borderColor			  = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
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

		uint32 seconds = (uint32)Application::Get().GetTime();

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
		float time = Application::Get().GetTime();

		Pixel* imageData = m_ImageData;
		for (uint32 y = 0u; y < 16u; y++) 
		{
			for (uint32 x = 0u; x < 16u; x++) 
			{
				float r = 255.0f * (0.5f + 0.5f * std::sin(3 * time));
				imageData[x] = Pixel{ uint8(r),uint8(y * 16u),uint8(x * 16u),255u};
			}
			imageData += m_ImageRowPitch / sizeof(Pixel);
		}

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

		vmaDestroyImage(allocator, m_Image, m_ImageAllocation);
		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}
}