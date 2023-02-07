#include "TestLayer.h"
#include "Core/Application.h"

namespace DT
{
	void TestLayer::OnAttach()
	{
		LOG_TRACE("Attached!");
	}

	void TestLayer::OnUpdate(float dt)
	{
	}

	void TestLayer::OnEvent(Event& event)
	{
	}

	void TestLayer::OnDetach()
	{
		LOG_TRACE("Detached!");
	}
}