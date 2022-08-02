#include <imgui/imgui.h>
#include "Gui/Controls/ColorEditor.h"

namespace GUI::CONTROLS
{
    /// Displays the color editor and allows a user to edit the color.
    /// @param[in]  color_label - The label for the color to display in the editor.
    /// @param[in,out]  color - The color to display and allowing editing of.
    void ColorEditor::DisplayAndAllowEditing(const char* const color_label, GRAPHICS::Color& color)
    {
        float color_components[4] = { color.Red, color.Green, color.Blue, color.Alpha };
        if (ImGui::ColorEdit4(color_label, color_components))
        {
            color.Red = color_components[0];
            color.Green = color_components[1];
            color.Blue = color_components[2];
            color.Alpha = color_components[3];
        }
    }
}
