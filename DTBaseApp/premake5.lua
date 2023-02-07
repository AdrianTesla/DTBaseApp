project "DTBaseApp"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	ignoredefaultlibraries { "LIBCMT" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-intermediate/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/DTBaseApp/src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.VulkanSDK}"
	}

	libdirs 
	{
		""
	}

	links
	{
		"GLFW",
		"%{Library.Vulkan}"
	}

	postbuildcommands 
	{
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"DT_PLATFORM_WINDOWS",
			"_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS",
			"DT_ENABLE_LOGGING"
		}

	filter "configurations:Debug"
		defines "DT_DEBUG"
		runtime "Debug"
		symbols "on"
		kind "ConsoleApp"

		links
		{
			"%{Library.Vulkan}"
		}

	filter "configurations:Release"
		defines "DT_RELEASE"
		runtime "Release"
		optimize "on"
		kind "ConsoleApp"

		links
		{
		}












		filter "configurations:Debug"
		defines "DT_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
		  	"%{Library.Vulkan}",
		  	"%{Library.ShaderC_Debug}",
		  	"%{Library.SPIRV_Tools_Debug}",
		  	"%{Library.SPIRV_Cross_Debug}",
		  	"%{Library.SPIRV_Cross_GLSL_Debug}",
		  	"%{Library.SPIRV_Cross_HLSL_Debug}"
		}

	filter "configurations:Release"
		defines "DT_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.Vulkan}",
		  	"%{Library.ShaderC_Release}",
		  	"%{Library.SPIRV_Cross_Release}",
		  	"%{Library.SPIRV_Tools_Release}",
		  	"%{Library.SPIRV_Cross_GLSL_Release}",
		  	"%{Library.SPIRV_Cross_HLSL_Release}"
		}