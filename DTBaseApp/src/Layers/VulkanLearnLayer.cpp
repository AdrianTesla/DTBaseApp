#include "VulkanLearnLayer.h"
#include "Core/Application.h"

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
		LOG_TRACE("Attached!");
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
	}

	void VulkanLearnLayer::OnDetach()
	{
		LOG_TRACE("Detached!");
	}
}