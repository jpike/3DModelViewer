#pragma once

#include "Graphics/Material.h"

namespace GUI::PANELS
{
    /// A panel for viewing/editing materials.
    class MaterialPanel
    {
    public:
        static void UpdateAndRender(GRAPHICS::Material& material);
    };
}
