#include <imgui/imgui.h>
#include "Gui/Controls/ColorEditor.h"
#include "Gui/Panels/MaterialPanel.h"

namespace GUI::PANELS
{
    /// Updates and renders the panel.
    /// @param[in,out]  material - The material to display and potentially update in the panel.
    void MaterialPanel::UpdateAndRender(GRAPHICS::Material& material)
    {
        // DISPLAY THE MATERIAL NAME.
        std::string material_label = "Material: " + material.Name;
        ImGui::Text(material_label.c_str());

        // ALLOW EDITING THE SHADING TYPE.
        bool wireframe_configured = (GRAPHICS::SHADING::ShadingType::WIREFRAME == material.Shading);
        if (ImGui::RadioButton("WIREFRAME", wireframe_configured))
        {
            material.Shading = GRAPHICS::SHADING::ShadingType::WIREFRAME;
        }
        bool flat_configured = (GRAPHICS::SHADING::ShadingType::FLAT == material.Shading);
        if (ImGui::RadioButton("FLAT", flat_configured))
        {
            material.Shading = GRAPHICS::SHADING::ShadingType::FLAT;
        }
        bool material_configured = (GRAPHICS::SHADING::ShadingType::MATERIAL == material.Shading);
        if (ImGui::RadioButton("MATERIAL", material_configured))
        {
            material.Shading = GRAPHICS::SHADING::ShadingType::MATERIAL;
        }

        // ALLOW EDITING AMBIENT PROPERTIES.
        CONTROLS::ColorEditor::DisplayAndAllowEditing("Ambient Color", material.AmbientProperties.Color);

        // ALLOW EDITING DIFFUSE PROPERTIES.
        CONTROLS::ColorEditor::DisplayAndAllowEditing("Diffuse Color", material.DiffuseProperties.Color);

        // ALLOW EDITING SPECULAR PROPERTIES.
        CONTROLS::ColorEditor::DisplayAndAllowEditing("Specular Color", material.SpecularProperties.Color);
        ImGui::SliderFloat("Specular Power", (float*)&material.SpecularProperties.SpecularPower, 0.0f, 100.0f);

        // ALLOW EDITING THE REFLECTIVITY.
        ImGui::SliderFloat("Reflectivity", (float*)&material.ReflectivityProportion, 0.0f, 1.0f);

        // ALLOW EDITING THE EMISSIVE COLOR.
        CONTROLS::ColorEditor::DisplayAndAllowEditing("Emissive Color", material.EmissiveColor);
    }
}
