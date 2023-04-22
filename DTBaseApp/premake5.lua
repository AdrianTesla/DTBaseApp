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
		"src/**.cpp",
		"%{wks.location}/vendor/stb/**.h"
	}

	includedirs
	{
		"%{wks.location}/DTBaseApp/src",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.glm}"
	}

	libdirs 
	{
	}

	links
	{
		"GLFW"
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
		}

	filter "configurations:Release"
		defines "DT_RELEASE"
		runtime "Release"
		optimize "on"
		kind "ConsoleApp"

		links
		{
		}