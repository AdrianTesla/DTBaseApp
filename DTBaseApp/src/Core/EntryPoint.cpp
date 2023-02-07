#include "Application.h"

#if DT_PLATFORM_WINDOWS

int main()
{
	DT::InitializeCore();

	DT::Application* app = new DT::Application({});
	app->Run();
	delete app;

	DT::ShutdownCore();

	/* Memory Leak Detector: for some reason, spdlog leaks. So disable before testing */
	
	/*
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		if (_CrtDumpMemoryLeaks())
			__debugbreak();
	*/

	return 0;
}

#else
	#error Only Windows Supported!
#endif