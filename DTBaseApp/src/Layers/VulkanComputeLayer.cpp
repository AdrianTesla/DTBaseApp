#include "VulkanComputeLayer.h"
#include "Core/Application.h"
#include "Platform/API_Vulkan/VulkanContext.h"
#include "Platform/API_Vulkan/VulkanRenderer.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer/Renderer.h"

namespace DT
{
	static struct ComputeUniformBufferData
	{
		float Time = 0.0f;
		float DeltaTime = 0.0;
	} s_ComputeUniformBuffer{};

	void VulkanComputeLayer::OnAttach()
	{
		constexpr glm::vec3 colors[] = {
			{ 1.0f,0.0f,0.0f },
			{ 0.0f,1.0f,0.0f },
			{ 0.0f,0.0f,1.0f },
			{ 1.0f,1.0f,0.0f },
			{ 1.0f,0.0f,1.0f },
			{ 0.0f,1.0f,1.0f },
			{ 1.0f,1.0f,1.0f },
			{ 0.5f,0.5f,0.5f }
		};

		struct Particle
		{
			glm::vec2 Position;
			glm::vec2 Velocity;
			glm::vec3 Color;
		};

		static Particle particles[100];
		m_ParticleBufferSize = sizeof(particles);

		auto rng = []()
		{
			return 2.0f * rand() / (float)RAND_MAX - 1.0f;
		};

		for (uint32 i = 0u; i < std::size(particles); i++)
		{
			particles[i].Position = { rng(),rng() };
			particles[i].Velocity = { rng(),rng() };
			particles[i].Color = colors[i % std::size(colors)];
		}

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		// create staging buffer to hold the particle initial conditions
		VulkanBuffer stagingBuffer;
		VmaAllocationCreateInfo stagingAllocationCreateInfo{};
		stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		stagingAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(m_ParticleBufferSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, &stagingAllocationCreateInfo, &stagingBuffer);
			
		// upload CPU particles to the staging buffer
		ASSERT(stagingBuffer.AllocationInfo.pMappedData != nullptr);
		memcpy(stagingBuffer.AllocationInfo.pMappedData, &particles, m_ParticleBufferSize);

		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Transfer);

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			// create device local buffer (vertex buffer | storage buffer | transfer dst)
			VmaAllocationCreateInfo allocationCreateInfo{};
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			allocationCreateInfo.flags = 0u;
			Vulkan::CreateBuffer(
				m_ParticleBufferSize, 
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
				&allocationCreateInfo, 
				&m_ParticleBuffer[i]
			);

			// copy the particle data from the staging buffer to this GPU dual-usage buffer
			vkCmd::CopyBuffer(commandBuffer, stagingBuffer.Buffer, m_ParticleBuffer[i].Buffer, m_ParticleBufferSize);
		}
		vulkanDevice.EndCommandBuffer();
		Vulkan::DestroyBuffer(stagingBuffer);

		m_ComputePipeline = Ref<VulkanComputePipeline>::Create();

		// create graphics command buffers
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

		// create uniform buffer
		m_UniformBuffer = Ref<VulkanUniformBuffer>::Create(sizeof(ComputeUniformBufferData));

		// create descriptor pool
		{
			VkDevice device = VulkanContext::GetCurrentVulkanDevice();

			VkDescriptorPoolSize descriptorPoolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,MAX_FRAMES_IN_FLIGHT },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,MAX_FRAMES_IN_FLIGHT * 2u }
			};

			VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
			descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCreateInfo.pNext         = nullptr;
			descriptorPoolCreateInfo.flags         = 0u;
			descriptorPoolCreateInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
			descriptorPoolCreateInfo.poolSizeCount = (uint32)std::size(descriptorPoolSizes);
			descriptorPoolCreateInfo.pPoolSizes    = descriptorPoolSizes;
			VK_CALL(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool));
		}

		// create descriptor sets
		{
			VkDescriptorSetLayout descriptorSetLayout = m_ComputePipeline->GetDescriptorSetLayout();
			VkDescriptorSetLayout descriptorSetLayouts[MAX_FRAMES_IN_FLIGHT];
			for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
				descriptorSetLayouts[i] = descriptorSetLayout;

			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType				 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.pNext				 = nullptr;
			descriptorSetAllocateInfo.descriptorPool	 = m_DescriptorPool;
			descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
			descriptorSetAllocateInfo.pSetLayouts		 = descriptorSetLayouts;
			VK_CALL(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, m_DescriptorSets.Data()));
		}

		// link the resources to the descriptor sets
		{
			for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				std::array<VkWriteDescriptorSet, 3u> writeDescriptorSets;

				// uniform buffer
				writeDescriptorSets[0].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].pNext			= nullptr;
				writeDescriptorSets[0].dstSet			= m_DescriptorSets[i];
				writeDescriptorSets[0].dstBinding		= 0u;
				writeDescriptorSets[0].dstArrayElement	= 0u;
				writeDescriptorSets[0].descriptorCount	= 1u;
				writeDescriptorSets[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pImageInfo		= nullptr;
				writeDescriptorSets[0].pBufferInfo		= &m_UniformBuffer->GetDescriptorBufferInfo();
				writeDescriptorSets[0].pTexelBufferView	= nullptr;

				VkDescriptorBufferInfo prevFrameDescriptorBufferInfo{};
				prevFrameDescriptorBufferInfo.buffer = m_ParticleBuffer[(i - 1u) % MAX_FRAMES_IN_FLIGHT].Buffer;
				prevFrameDescriptorBufferInfo.offset = 0u;
				prevFrameDescriptorBufferInfo.range  = m_ParticleBufferSize;

				// prev frame storage buffer
				writeDescriptorSets[1].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[1].pNext			= nullptr;
				writeDescriptorSets[1].dstSet			= m_DescriptorSets[i];
				writeDescriptorSets[1].dstBinding		= 1u;
				writeDescriptorSets[1].dstArrayElement	= 0u;
				writeDescriptorSets[1].descriptorCount	= 1u;
				writeDescriptorSets[1].descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeDescriptorSets[1].pImageInfo		= nullptr;
				writeDescriptorSets[1].pBufferInfo		= &prevFrameDescriptorBufferInfo;
				writeDescriptorSets[1].pTexelBufferView	= nullptr;

				VkDescriptorBufferInfo currFrameDescriptorBufferInfo{};
				currFrameDescriptorBufferInfo.buffer = m_ParticleBuffer[i].Buffer;
				currFrameDescriptorBufferInfo.offset = 0u;
				currFrameDescriptorBufferInfo.range	 = m_ParticleBufferSize;

				// current frame storage buffer
				writeDescriptorSets[2].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[2].pNext			= nullptr;
				writeDescriptorSets[2].dstSet			= m_DescriptorSets[i];
				writeDescriptorSets[2].dstBinding		= 2u;
				writeDescriptorSets[2].dstArrayElement	= 0u;
				writeDescriptorSets[2].descriptorCount	= 1u;
				writeDescriptorSets[2].descriptorType	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeDescriptorSets[2].pImageInfo		= nullptr;
				writeDescriptorSets[2].pBufferInfo		= &currFrameDescriptorBufferInfo;
				writeDescriptorSets[2].pTexelBufferView	= nullptr;

				vkUpdateDescriptorSets(device, (uint32)writeDescriptorSets.size(), writeDescriptorSets.data(), 0u, nullptr);
			}
		}
	}

	void VulkanComputeLayer::OnUpdate(float dt)
	{
		s_ComputeUniformBuffer.Time += dt;
		s_ComputeUniformBuffer.DeltaTime = dt;
		m_UniformBuffer->SetData(&s_ComputeUniformBuffer, sizeof(s_ComputeUniformBuffer));
	}

	void VulkanComputeLayer::OnRender()
	{
		VkCommandBuffer commandBuffer = m_GraphicsCommandBuffers[Renderer::CurrentFrame()];

		// record work
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo{};
			commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.pNext            = nullptr;
			commandBufferBeginInfo.flags            = 0u;
			commandBufferBeginInfo.pInheritanceInfo = nullptr;

			VkDeviceSize offsets = 0u;

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->GetVulkanPipeline());
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->GetPipelineLayout(), 0u, 1u, &m_DescriptorSets[Renderer::CurrentFrame()], 0u, nullptr);
			}
			VK_CALL(vkEndCommandBuffer(commandBuffer));
		}

		// submit work
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
	}

	void VulkanComputeLayer::OnEvent(Event& event)
	{
	}

	void VulkanComputeLayer::OnDetach()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VK_CALL(vkDeviceWaitIdle(device));

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
			Vulkan::DestroyBuffer(m_ParticleBuffer[i]);
		}

		VK_CALL(vkDeviceWaitIdle(device));
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}