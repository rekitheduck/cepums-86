-- Cepums-86 emulator premake5 script

workspace "Cepums-86"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories
includeDir = {}
includeDir["spdlog"] = "subprojects/spdlog/include"
includeDir["SDL"] = "subprojects/SDL/SDL-repo/include"

-- spdlog project premake
project "spdlog"
    location "subprojects/spdlog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("temp/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.location}/**.h"
    }

    includedirs
    {
        "%{includeDir.spdlog}"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

-- Cepums-86 project
project "Cepums-86"
    location "cepums"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("temp/" .. outputdir .. "/%{prj.name}")

    pchheader "cepumspch.h"
    pchsource "cepums/cepumspch.cpp"

    files
    {
        "%{prj.location}/**.h",
        "%{prj.location}/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "PREMAKE",
    }

    includedirs
    {
        "%{prj.location}",
        "%{includeDir.spdlog}",
        "%{includeDir.SDL}",
    }

    links
    {
        "SDL2"
    }

    libdirs
    {
        "subprojects/SDL/lib"
    }

    filter "system:linux"
        links
        {
            "pthread"
        }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines
        {
            "CEPUMS_DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "CEPUMS_RELEASE"
        runtime "Release"
        optimize "on"
