#include <imgui/imgui.h>
#include "Gui/Windows/RendererSettingsWindow.h"

namespace GUI::WINDOWS
{
    /// Updates and renders the window, if open.
    /// @param[in,out]  rendering_settings - The rendering settings to update/display in the window.
    /// @param[in,out]  graphics_device - The graphics device for which the rendering settings apply.
    void RendererSettingsWindow::UpdateAndRender(GRAPHICS::RenderingSettings& rendering_settings, GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device)
    {
        // DON'T RENDER THE WINDOW IF IT IS CLOSED.
        if (!IsOpen)
        {
            return;
        }

        // RENDER THE WINDOW.
        if (ImGui::Begin("Renderer", &IsOpen))
        {
            // ALLOW THE USER TO CHANGE THE BASIC TYPE OF GRAPHICS DEVICE.
            GRAPHICS::HARDWARE::GraphicsDeviceType current_graphics_device_type = graphics_device.Type();
            bool rasterization_configured = (GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER == current_graphics_device_type);
            if (ImGui::RadioButton("CPU RASTERIZER", rasterization_configured))
            {
                rendering_settings.GraphicsDeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RASTERIZER;
            }

            bool ray_tracing_configured = (GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER == current_graphics_device_type);
            if (ImGui::RadioButton("CPU RAY TRACER", ray_tracing_configured))
            {
                rendering_settings.GraphicsDeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER;
            }

            bool open_gl_configured = (GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL == current_graphics_device_type);
            if (ImGui::RadioButton("OPEN GL", open_gl_configured))
            {
                rendering_settings.GraphicsDeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::OPEN_GL;
            }

            bool direct_3d_configured = (GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D == current_graphics_device_type);
            if (ImGui::RadioButton("DIRECT 3D", direct_3d_configured))
            {
                rendering_settings.GraphicsDeviceType = GRAPHICS::HARDWARE::GraphicsDeviceType::DIRECT_3D;
            }

            // ALLOW EDITING OTHER KINDS OF RENDERING SETTINGS.
            /// @todo   Figure out how to communicate that all settings are not applicable to all renderers.
            ImGui::Checkbox("CPU SIMD?", &rendering_settings.UseCpuSimd);
            ImGui::Checkbox("Cull Backfaces?", &rendering_settings.CullBackfaces);
            ImGui::Checkbox("Depth Buffering?", &rendering_settings.DepthBuffering);
            ImGui::Checkbox("Lighting?", &rendering_settings.Shading.Lighting.Enabled);
            ImGui::Checkbox("Render point lights?", &rendering_settings.Shading.Lighting.RenderPointLights);
            ImGui::Checkbox("Ambient Lighting?", &rendering_settings.Shading.Lighting.AmbientLightingEnabled);
            ImGui::Checkbox("Shadows?", &rendering_settings.Shading.Lighting.ShadowsEnabled);
            ImGui::Checkbox("Diffuse Shading?", &rendering_settings.Shading.Lighting.DiffuseLightingEnabled);
            ImGui::Checkbox("Specular Shading?", &rendering_settings.Shading.Lighting.SpecularLightingEnabled);
            ImGui::Checkbox("Reflections?", &rendering_settings.Reflections);
            ImGui::SliderInt("Max Reflection Count:", reinterpret_cast<int*>(&rendering_settings.MaxReflectionCount), 0, 30);
            ImGui::Checkbox("Texture Mapping?", &rendering_settings.Shading.TextureMappingEnabled);

            // ALLOW THE USER TO CHANGE THE SHADING TYPE.
            bool wireframe_configured = (GRAPHICS::SHADING::ShadingType::WIREFRAME == rendering_settings.Shading.ShadingType);
            if (ImGui::RadioButton("WIREFRAME", wireframe_configured))
            {
                rendering_settings.Shading.ShadingType = GRAPHICS::SHADING::ShadingType::WIREFRAME;
            }
            bool flat_configured = (GRAPHICS::SHADING::ShadingType::FLAT == rendering_settings.Shading.ShadingType);
            if (ImGui::RadioButton("FLAT", flat_configured))
            {
                rendering_settings.Shading.ShadingType = GRAPHICS::SHADING::ShadingType::FLAT;
            }
            bool material_configured = (GRAPHICS::SHADING::ShadingType::MATERIAL == rendering_settings.Shading.ShadingType);
            if (ImGui::RadioButton("MATERIAL", material_configured))
            {
                rendering_settings.Shading.ShadingType = GRAPHICS::SHADING::ShadingType::MATERIAL;
            }
        }
        ImGui::End();
    }
}