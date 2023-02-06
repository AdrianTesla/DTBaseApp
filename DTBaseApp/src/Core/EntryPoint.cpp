#include "Application.h"

#if DT_PLATFORM_WINDOWS

int main()
{
	DT::InitializeCore();

	DT::Application* app = new DT::Application();
	app->Run();
	delete app;

	DT::ShutdownCore();
	return 0;
}

#else
	#error Only Windows Supported!
#endif