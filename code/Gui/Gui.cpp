#include <commdlg.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_sw.hpp>
#include <imgui/imgui.h>
#include "ErrorHandling/Asserts.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/DirectX/Direct3DGraphicsDevice.h"
#include "Gui/Gui.h"
#include "Windowing/Win32Window.h"

namespace GUI
{
    /// Attempts to create the GUI for rendering using the specified device to the specified window.
    /// @param[in]  graphics_device - The graphics device to use for rendering the GUI.
    /// @param[in]  window - The window in which to render the GUI.
    /// @return The GUI, if successfully created; null otherwise.
    std::optional<Gui> Gui::Create(
        const GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device,
        const WINDOWING::IWindow& window)
    {
        // TRY INITIALIZING COMPONENTS OF THE IMGUI LIBRARY.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        const WINDOWING::Win32Window& win32_window = dynamic_cast<const WINDOWING::Win32Window&>(window);
        bool imgui_initialized = ImGui_ImplWin32_Init(win32_window.WindowHandle);
        ASSERT_THEN_IF(imgui_initialized)
        {
            // INITIALIZE PARTS OF THE IMGUI LIBRARY BASED ON THE TYPE OF GRAPHICS DEVICE.
            GRAPHICS::HARDWARE::GraphicsDeviceType current_graphics_device_type = graphics_device.Type();
            switch (current_graphics_device_type)
            {
                case GRAPHICS::HARDWARE::CPU_RASTERIZER:
                case GRAPHICS::HARDWARE::CPU_RAY_TRACER:
                {
                    imgui_sw::bind_imgui_painting();
                    break;
                }
                case GRAPHICS::HARDWARE::OPEN_GL:
                {
                    bool imgui_open_gl_initializated = ImGui_ImplOpenGL3_Init();
                    ASSERT_THEN_IF_NOT(imgui_open_gl_initializated)
                    {
                        return std::nullopt;
                    }
                    break;
                }
                case GRAPHICS::HARDWARE::DIRECT_3D:
                {
                    const GRAPHICS::DIRECT_X::Direct3DGraphicsDevice& direct_x_graphics_device = dynamic_cast<const GRAPHICS::DIRECT_X::Direct3DGraphicsDevice&>(graphics_device);
                    bool imgui_direct_x_initialized = ImGui_ImplDX11_Init(direct_x_graphics_device.Device, direct_x_graphics_device.DeviceContext);
                    ASSERT_THEN_IF_NOT(imgui_direct_x_initialized)
                    {
                        return std::nullopt;
                    }
                    break;
                }
            }
            
            Gui gui;
            return gui;
        }
        else
        {
            return std::nullopt;
        }
    }

    /// Updates and renders the GUI.
    /// @param[in,out]  graphics_device - The graphics device to use for rendering the GUI.
    /// @param[in,out]  scene - The scene being controlled by the GUI.
    /// @param[in,out]  camera - The camera through which the scene is being viewed.
    /// @param[in,out]  rendering_settings - The settings for rendering to potentially update.
    void Gui::UpdateAndRender(
        GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device, 
        GRAPHICS::Scene& scene,
        GRAPHICS::VIEWING::Camera& camera,
        GRAPHICS::RenderingSettings& rendering_settings)
    {
        // START THE NEW FRAME.
        ImGui_ImplWin32_NewFrame();

        GRAPHICS::HARDWARE::GraphicsDeviceType current_graphics_device_type = graphics_device.Type();
        switch (current_graphics_device_type)
        {
            // Only a subset of types of graphics devices require special handling at this stage.
            case GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL:
            {
                ImGui_ImplOpenGL3_NewFrame();
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D:
            {
                ImGui_ImplDX11_NewFrame();
                break;
            }
        }

        ImGui::NewFrame();

        // UPDATE AND RENDER THE MAIN MENU.
        SelectedFilepath.clear();;
        if (ImGui::BeginMainMenuBar())
        {
            // UPDATE AND RENDER THE FILE MENU.
            if (ImGui::BeginMenu("File"))
            {
                // HAVE A MENU ITEM FOR OPENING A FILE.
                if (ImGui::MenuItem("Open"))
                {
                    // PROMPT THE USER TO SELECT A FILE.
                    // Even though this older API (https://docs.microsoft.com/en-us/windows/win32/dlgbox/open-and-save-as-dialog-boxes)
                    // has been superseded by a newer API, the newer API (https://docs.microsoft.com/en-us/windows/win32/shell/common-file-dialog)
                    // is way more complicated to use, so it is not used instead.
                    // See https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for max path limitations.
                    constexpr std::size_t WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS = 32767;
                    char chosen_filepath[WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS] = {};
                    OPENFILENAME open_file_dialog_settings =
                    {
                        .lStructSize = sizeof(OPENFILENAME),
                        .hwndOwner = NULL, // No owner.
                        .hInstance = NULL, // No special template for the dialog box.
                        .lpstrFilter = NULL, // No filtering down types of files.
                        .lpstrCustomFilter = NULL, // No preservation of custom user-selected filters.
                        .nMaxCustFilter = 0, // User-selected custom filters are not being used.
                        .nFilterIndex = 0, // Filters are not being used.
                        .lpstrFile = chosen_filepath, // Buffer to be populated with filepath.
                        .nMaxFile = WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS, // Size of filepath buffer.
                        .lpstrFileTitle = NULL, // No initial filename and extension.
                        .nMaxFileTitle = 0, // Ignored since no initial filename/extension.
                        .lpstrInitialDir = NULL, // No custom initial directory.  The exact initial directory will vary by platform.
                        .lpstrTitle = NULL, // Use default "Open" title.
                        .Flags = (
                            // Allow resizing.
                            OFN_ENABLESIZING |
                            // Selected files must actually exist.
                            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                            // Ensure that longer filepaths can be supported.
                            OFN_LONGNAMES),
                        .nFileOffset = 0, // Will be populated with offset from path to filename.
                        .nFileExtension = 0, // Will be populated with offset from path to file extension.
                        .lpstrDefExt = NULL, // No default extension.
                        .lCustData = NULL, // No custom data.
                        .lpfnHook = NULL, // No custom hook.
                        .lpTemplateName = NULL, // No dialog template.
                        // Remaining members are reserved and thus not specified here.
                    };
                    BOOL file_chosen = GetOpenFileName(&open_file_dialog_settings);
                    if (file_chosen)
                    {
                        SelectedFilepath = chosen_filepath;
                    }
                }
                ImGui::EndMenu();
            }

            // UPDATE AND RENDER THE SETTINGS MENU.
            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::MenuItem("Renderer"))
                {
                    RendererSettingsWindow.IsOpen = true;
                }

                if (ImGui::MenuItem("Camera"))
                {
                    CameraWindow.IsOpen = true;
                }

                ImGui::EndMenu();
            }

            // UPDATE AND RENDER THE SCENE MENU.
            if (ImGui::BeginMenu("Scene"))
            {
                if (ImGui::MenuItem("Current"))
                {
                    SceneWindow.IsOpen = true;
                }
                ImGui::EndMenu();
            }

            // UPDATE AND RENDER A DEBUG MENU.
            if (ImGui::BeginMenu("Debug"))
            {
                if (ImGui::MenuItem("Metrics Window"))
                {
                    ImGuiMetricsWindowOpen = true;
                }
                if (ImGui::MenuItem("Demo Window"))
                {
                    ImGuiDemoWindowOpen = true;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // RENDER THE VARIOUS WINDOWS IF APPLICABLE.
        GRAPHICS::HARDWARE::GraphicsDeviceType old_renderer_type = rendering_settings.GraphicsDeviceType;

        RendererSettingsWindow.UpdateAndRender(rendering_settings, graphics_device);
        CameraWindow.UpdateAndRender(camera);

        SceneWindow.UpdateAndRender(scene);

        if (ImGuiDemoWindowOpen)
        {
            ImGui::ShowDemoWindow(&ImGuiDemoWindowOpen);
        }

        if (ImGuiMetricsWindowOpen)
        {
            ImGui::ShowMetricsWindow(&ImGuiMetricsWindowOpen);
        }

        // RENDER THE FRAME.
        ImGui::Render();

        switch (old_renderer_type)
        {
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER:
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER:
            {
                GRAPHICS::CPU_RENDERING::CpuGraphicsDevice& cpu_graphics_device = dynamic_cast<GRAPHICS::CPU_RENDERING::CpuGraphicsDevice&>(graphics_device);
                uint32_t* pixel_buffer = cpu_graphics_device.ColorBuffer.GetRawData();
                int pixel_buffer_width_in_pixels = static_cast<int>(cpu_graphics_device.ColorBuffer.GetWidthInPixels());
                int pixel_buffer_height_in_pixels = static_cast<int>(cpu_graphics_device.ColorBuffer.GetHeightInPixels());
                imgui_sw::paint_imgui(pixel_buffer, pixel_buffer_width_in_pixels, pixel_buffer_height_in_pixels);
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL:
            {
                ImDrawData* gui_draw_data = ImGui::GetDrawData();
                ImGui_ImplOpenGL3_RenderDrawData(gui_draw_data);
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D:
            {
                ImDrawData* gui_draw_data = ImGui::GetDrawData();
                ImGui_ImplDX11_RenderDrawData(gui_draw_data);
                break;
            }
        }

        // SHUTDOWN IMGUI COMPONENTS IF THE TYPE OF GRAPHICS DEVICE IS CHANGING.
        bool graphics_device_type_changed = (old_renderer_type != rendering_settings.GraphicsDeviceType);
        if (graphics_device_type_changed)
        {
            switch (old_renderer_type)
            {
                case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER:
                case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER:
                {
                    imgui_sw::unbind_imgui_painting();
                    break;
                }
                case GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL:
                {
                    ImGui_ImplOpenGL3_Shutdown();
                    break;
                }
                case GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D:
                {
                    ImGui_ImplDX11_Shutdown();
                    break;
                }
            }

            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }

    /// Shuts down the GUI.
    /// @param[in]  graphics_device_type - The type of graphics device the GUI is rendering to.
    void Gui::Shutdown(const GRAPHICS::HARDWARE::GraphicsDeviceType graphics_device_type)
    {
        switch (graphics_device_type)
        {
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER:
            case GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER:
            {
                imgui_sw::unbind_imgui_painting();
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL:
            {
                ImGui_ImplOpenGL3_Shutdown();
                break;
            }
            case GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D:
            {
                ImGui_ImplDX11_Shutdown();
                break;
            }
        }

        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
