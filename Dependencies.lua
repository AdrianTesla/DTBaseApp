VULKAN_SDK = os.getenv("VULKAN_SDK")

-- include directories
IncludeDir = {}
IncludeDir["glfw"]         = "%{wks.location}/vendor/glfw/include"
IncludeDir["spdlog"]       = "%{wks.location}/vendor/spdlog/include"
IncludeDir["vk_mem_alloc"] = "%{wks.location}/vendor/vk_mem_alloc"
IncludeDir["stb"]          = "%{wks.location}/vendor/stb"
IncludeDir["glm"]          = "%{wks.location}/vendor/glm"
IncludeDir["zlib"]          = "%{wks.location}/vendor/zlib/include"
IncludeDir["Assimp"]       = "%{wks.location}/vendor/assimp/include"
IncludeDir["VulkanSDK"]    = "%{VULKAN_SDK}/Include"

-- library directories
LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

-- libraries --
Library = {}

Library["Vulkan"]      = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["zlib_Debug"]             = "%{wks.location}/vendor/zlib/lib/Debug/zlibstaticd.lib"
Library["Assimp_Debug"]           = "%{wks.location}/vendor/assimp/lib/Debug/assimp-vc143-mtd.lib"
Library["ShaderC_Debug"]          = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Tools_Debug"]      = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"
Library["SPIRV_Cross_Debug"]      = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Cross_HLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-hlsld.lib"

Library["zlib_Release"]             = "%{wks.location}/vendor/zlib/lib/Release/zlibstatic.lib"
Library["Assimp_Release"]           = "%{wks.location}/vendor/assimp/lib/Release/assimp-vc143-mt.lib"
Library["ShaderC_Release"]          = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Tools_Release"]      = "%{LibraryDir.VulkanSDK}/SPIRV-Tools.lib"
Library["SPIRV_Cross_Release"]      = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
Library["SPIRV_Cross_HLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-hlsl.lib"