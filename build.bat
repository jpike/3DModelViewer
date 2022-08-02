#if 0

@ECHO OFF

REM MAKE SURE THE COMPILER IS CONFIGURED FOR x64.
REM This currently cannot be done via the C++ build system as just using std::system() to execute these batch files
REM does not result in appropriate environment variables being preserved.
IF NOT DEFINED VCINSTALLDIR (
    ECHO "Visual Studio tools not configured...Configuring for x64..."
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    ECHO "Incorrect Visual Studio target architecture...Reconfiguring for x64..."
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

ECHO "Double-checking for compiler..."
WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
WHERE cl.exe

REM BUILD THE C++ BUILD PROGRAM.
REM For exception handling flag - https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-160
REM /TP is needed to have this batch file treated as a .cpp file - https://docs.microsoft.com/en-us/cpp/build/reference/tc-tp-tc-tp-specify-source-file-type?view=msvc-160
cl.exe /std:c++latest /EHsc /I ..\..\CppLibraries /TP build.bat

REM BUILD THE PROJECT
build.exe

COPY ..\..\CppLibraries\ThirdParty\SDL\SDL2.dll build\debug\SDL2.dll
COPY ..\..\CppLibraries\ThirdParty\SDL\SDL2.dll build\release\SDL2.dll

@ECHO ON
EXIT /B

#endif

#include <cstdlib>
#include <filesystem>
#include "BuildSystem/CppBuild.cpp"

int main()
{
    // DEFINE THE PATH TO THE WORKSPACE.
    // The path is converted to an absolute path to ensure it remains correct in all commands.
    std::filesystem::path workspace_folder_path = std::filesystem::absolute(".");

    // DEFINE THE BUILD TO ADD PROJECTS TO.
    Build build;

//#define DEBUG_BUILD 1
#define RELEASE_BUILD 1

    // DEFINE PROJECTS TO BUILD.
    Project model_viewer = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "3DModelViewer",
        .CodeFolderPath = workspace_folder_path / "code",
        .UnityBuildFilepath = workspace_folder_path / "3DModelViewer.project",
        .AdditionalIncludeFolderPaths = 
        {
            workspace_folder_path / "../../CppLibraries",
            workspace_folder_path / "../../CppLibraries/ThirdParty",
            workspace_folder_path / "../../CppLibraries/ThirdParty/gl3w",
            workspace_folder_path / "../../CppLibraries/ThirdParty/imgui",
            workspace_folder_path / "../../CppLibraries/ThirdParty/SDL",
        },
        .AdditionalLibraryFolderPaths = 
        {
            workspace_folder_path / "../../CppLibraries/ThirdParty/SDL",
            /// @todo   Allow basing these paths on build variant?
#if DEBUG_BUILD
            workspace_folder_path / "../../CppLibraries/build/debug",
#endif
#if RELEASE_BUILD
            workspace_folder_path / "../../CppLibraries/build/release",
#endif
        },
        .LinkerLibraryNames = 
        {
            "comdlg32.lib",
            "Filesystem.lib",
            "gdi32.lib",
            "gl3w.lib",
            "Graphics.lib",
            "ImGui.lib",
            "Math.lib",
            "opengl32.lib",
            "SDL2.lib", 
            "SDL2main.lib",
            "String.lib",
            "stb.lib",
            "d3d11.lib", 
            "dxgi.lib", 
            "d3dcompiler.lib",
            "Windowing.lib",
            "user32.lib", 
        },
    };
    build.Add(&model_viewer);

    // BUILD DEBUG VERSIONS OF THE PROJECTS.
#if DEBUG_BUILD
    int debug_build_exit_code = build.Run(workspace_folder_path, "debug");
    return debug_build_exit_code;
#endif

#if MULTIPLE_BUILDS
    bool debug_build_succeeded = (EXIT_SUCCESS == debug_build_exit_code);
    if (!debug_build_succeeded)
    {
        return debug_build_exit_code;
    }
#endif

#if RELEASE_BUILD
    // BUILD RELEASE VERSIONS OF THE PROJECT.
    int release_build_exit_code = build.Run(workspace_folder_path, "release");
    return release_build_exit_code;
#endif
}
