include "Dependencies.lua"

workspace "DTBaseApp"
	architecture "x86_64"
	startproject "DTBaseApp"
	
	configurations 
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "DTBaseApp"

group "Dependencies"
	include "vendor/glfw"
group ""