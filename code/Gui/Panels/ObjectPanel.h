#pragma once

#include "Graphics/Object3D.h"

namespace GUI::PANELS
{
    /// A panel for viewing/editing 3D objects in the scene.
    class ObjectPanel
    {
    public:
        static void UpdateAndRender(GRAPHICS::Object3D& object);
    };
}
