#pragma once

#include "Graphics/Color.h"

namespace GUI::CONTROLS
{
    /// A widget that allows editing a color.
    /// This class makes it easier to use the ImGui library with our custom Color data type.
    class ColorEditor
    {
    public:
        static void DisplayAndAllowEditing(const char* const color_label, GRAPHICS::Color& color);
    };
}
