#pragma once

#include "Graphics/Hardware/IGraphicsDevice.h"
#include "Graphics/RenderingSettings.h"

namespace GUI::WINDOWS
{
    /// The GUI window that lets users change rendering settings.
    class RendererSettingsWindow
    {
    public:
        // PUBLIC METHODS.
        void UpdateAndRender(GRAPHICS::RenderingSettings& rendering_settings, GRAPHICS::HARDWARE::IGraphicsDevice& graphics_device);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// True if the window is open; false if not.
        bool IsOpen = false;
    };
}
