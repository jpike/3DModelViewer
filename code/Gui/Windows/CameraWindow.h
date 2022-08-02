#pragma once

#include "Graphics/Viewing/Camera.h"

namespace GUI::WINDOWS
{
    /// A window letting users change camera settings.
    class CameraWindow
    {
    public:
        // PUBLIC METHODS.
        void UpdateAndRender(GRAPHICS::VIEWING::Camera& camera);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// True if the window is open; false if not.
        bool IsOpen = false;
    };
}
