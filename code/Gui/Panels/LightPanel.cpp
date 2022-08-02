#include <imgui/imgui.h>
#include "Gui/Controls/ColorEditor.h"
#include "Gui/Panels/LightPanel.h"

namespace GUI::PANELS
{
    /// Updates and renders the panel.
    /// @param[in,out]  light - The light to display and potentially update in the panel.
    void LightPanel::UpdateAndRender(GRAPHICS::SHADING::LIGHTING::Light& light)
    {
        // ALLOW THE USER TO CHANGE THE LIGHT TYPE.
        bool is_ambient_light = (GRAPHICS::SHADING::LIGHTING::LightType::AMBIENT == light.Type);
        if (ImGui::RadioButton("AMBIENT", is_ambient_light))
        {
            light.Type = GRAPHICS::SHADING::LIGHTING::LightType::AMBIENT;
        }

        bool is_point_light = (GRAPHICS::SHADING::LIGHTING::LightType::POINT == light.Type);
        if (ImGui::RadioButton("POINT", is_point_light))
        {
            light.Type = GRAPHICS::SHADING::LIGHTING::LightType::POINT;
        }

        bool is_directional_light = (GRAPHICS::SHADING::LIGHTING::LightType::DIRECTIONAL == light.Type);
        if (ImGui::RadioButton("DIRECTIONAL", is_directional_light))
        {
            light.Type = GRAPHICS::SHADING::LIGHTING::LightType::DIRECTIONAL;
        }

        // ALLOW THE USER TO CHANGE THE LIGHT COLOR.
        CONTROLS::ColorEditor::DisplayAndAllowEditing("Color", light.Color);

        // ALLOW THE USER TO CHANGE THE LIGHT'S DIRECTION.
        /// @todo   Determine bounds more properly.
        ImGui::SliderFloat3("Direction (Directional Lights)", (float*)&light.DirectionalLightDirection, -50.0f, 50.0f);

        // ALLOW THE USER TO CHANGE THE LIGHT'S POSITION.
        /// @todo   Determine bounds more properly.
        ImGui::SliderFloat3("Position (Point Lights)", (float*)&light.PointLightWorldPosition, -50.0f, 50.0f);
    }
}
