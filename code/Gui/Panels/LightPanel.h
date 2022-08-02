#pragma once

#include "Graphics/Shading/Lighting/Light.h"

namespace GUI::PANELS
{
    /// A panel for viewing/editing information about a light.
    class LightPanel
    {
    public:
        static void UpdateAndRender(GRAPHICS::SHADING::LIGHTING::Light& light);
    };
}