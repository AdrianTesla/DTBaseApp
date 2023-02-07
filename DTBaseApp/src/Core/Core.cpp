#include "Core.h"

namespace DT
{
	void InitializeCore()
	{
		#if DT_ENABLE_LOGGING
			Log::Init();
		#endif
	}
	
	void ShutdownCore()
	{
		#if DT_ENABLE_LOGGING
			Log::Shutdown();
		#endif
	}
}