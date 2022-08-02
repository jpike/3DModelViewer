#pragma once

#include "Graphics/Scene.h"

namespace GUI::WINDOWS
{
    /// A window for viewing/editing information about the scene.
    class SceneWindow
    {
    public:
        // PUBLIC METHODS.
        void UpdateAndRender(GRAPHICS::Scene& scene);

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// True if the window is open; false if not.
        bool IsOpen = false;
    };
}
