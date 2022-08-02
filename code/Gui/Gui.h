#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/Object3D.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Scene.h"
#include "Gui/Windows/CameraWindow.h"
#include "Gui/Windows/RendererSettingsWindow.h"
#include "Gui/Windows/SceneWindow.h"
#include "Windowing/IWindow.h"

/// Holds code related to traditional Windows-Icons-Menus-Pointers (WIMP) style graphical user interfaces (GUIs).
/// See https://en.wikipedia.org/wiki/WIMP_(computing).
namespace GUI
{
    /// The main GUI for the 3D model viewer.
    class Gui
    {
    public:
        // CREATION.
        static std::optional<Gui> Create(
            const GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device,
            const WINDOWING::IWindow& window);

        // UPDATING METHODS.
        void UpdateAndRender(
            GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device, 
            GRAPHICS::Scene& scene,
            GRAPHICS::VIEWING::Camera& camera,
            GRAPHICS::RenderingSettings& rendering_settings);

        // SHUTDOWN METHODS.
        void Shutdown(const GRAPHICS::HARDWARE::GraphicsDeviceType graphics_device_type);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// Selected model filepath.
        std::filesystem::path SelectedFilepath = "";

        /// The window letting a user change rendering settings.
        WINDOWS::RendererSettingsWindow RendererSettingsWindow = {};
        /// The window letting a user change camera settings.
        WINDOWS::CameraWindow CameraWindow = {};
        /// The window letting a user view/edit scene information.
        WINDOWS::SceneWindow SceneWindow = {};

        /// True if the ImGui metrics window is open; false if not.
        bool ImGuiMetricsWindowOpen = false;
        /// True if the ImGui demo window is open; false if not.
        bool ImGuiDemoWindowOpen = false;
    };
}
