#include <string>
#include <optional>
#include <vector>
#include <commdlg.h>
#include <imgui/imgui.h>
#include "Graphics/Modeling/WavefrontObjectModel.h"
#include "Gui/Controls/ColorEditor.h"
#include "Gui/Panels/LightPanel.h"
#include "Gui/Panels/ObjectPanel.h"
#include "Gui/Windows/SceneWindow.h"

/// @todo   Figure out a better home for this.
std::string GetFilepathToOpenFromUser()
{
    // PROMPT THE USER TO SELECT A FILE.
    // Even though this older API (https://docs.microsoft.com/en-us/windows/win32/dlgbox/open-and-save-as-dialog-boxes)
    // has been superseded by a newer API, the newer API (https://docs.microsoft.com/en-us/windows/win32/shell/common-file-dialog)
    // is way more complicated to use, so it is not used instead.
    // See https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd for max path limitations.
    constexpr std::size_t WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS = 32767;
    char chosen_filepath[WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS] = {};
    OPENFILENAME open_file_dialog_settings =
    {
        .lStructSize = sizeof(OPENFILENAME),
        .hwndOwner = NULL, // No owner.
        .hInstance = NULL, // No special template for the dialog box.
        .lpstrFilter = NULL, // No filtering down types of files.
        .lpstrCustomFilter = NULL, // No preservation of custom user-selected filters.
        .nMaxCustFilter = 0, // User-selected custom filters are not being used.
        .nFilterIndex = 0, // Filters are not being used.
        .lpstrFile = chosen_filepath, // Buffer to be populated with filepath.
        .nMaxFile = WINDOWS_MAX_FILEPATH_LENGTH_IN_CHARACTERS, // Size of filepath buffer.
        .lpstrFileTitle = NULL, // No initial filename and extension.
        .nMaxFileTitle = 0, // Ignored since no initial filename/extension.
        .lpstrInitialDir = NULL, // No custom initial directory.  The exact initial directory will vary by platform.
        .lpstrTitle = NULL, // Use default "Open" title.
        .Flags = (
            // Allow resizing.
            OFN_ENABLESIZING |
            // Selected files must actually exist.
            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
            // Ensure that longer filepaths can be supported.
            OFN_LONGNAMES),
        .nFileOffset = 0, // Will be populated with offset from path to filename.
        .nFileExtension = 0, // Will be populated with offset from path to file extension.
        .lpstrDefExt = NULL, // No default extension.
        .lCustData = NULL, // No custom data.
        .lpfnHook = NULL, // No custom hook.
        .lpTemplateName = NULL, // No dialog template.
        // Remaining members are reserved and thus not specified here.
    };
    BOOL file_chosen = GetOpenFileName(&open_file_dialog_settings);
    if (file_chosen)
    {
        return chosen_filepath;
    }
    else
    {
        return "";
    }
}

namespace GUI::WINDOWS
{
    /// Updates and renders the window, if open.
    /// @param[in,out]  scene - The scene whose information to display (and possibly update).
    void SceneWindow::UpdateAndRender(GRAPHICS::Scene& scene)
    {
        // DON'T RENDER THE WINDOW IF IT IS CLOSED.
        if (!IsOpen)
        {
            return;
        }

        // RENDER THE WINDOW.
        if (ImGui::Begin("Scene", &IsOpen))
        {
            // ALLOW THE USER TO EDIT THE BACKGROUND COLOR.
            CONTROLS::ColorEditor::DisplayAndAllowEditing("Background Color", scene.BackgroundColor);

            // ALLOW THE USER TO VIEW/EDIT LIGHTS.
            /// @todo   Allow rendering of lights in scene!
            std::size_t light_count = scene.Lights.size();
            if (ImGui::TreeNode("Lights", "Lights (%zu)", light_count))
            {
                // LIST ALL LIGHTS.
                std::size_t light_index = 0;
                for (auto light = scene.Lights.begin(); light != scene.Lights.end(); ++light_index)
                {
                    // DISPLAY INFORMATION FOR THE CURRENT LIGHT.
                    bool light_removed = false;
                    std::string light_tree_label = "Light " + std::to_string(light_index);
                    if (ImGui::TreeNode(light_tree_label.c_str()))
                    {
                        // ALLOW THE USER TO REMOVE THE CURRENT LIGHT.
                        light_removed = ImGui::Button("Remove");

                        // ALLOW VIEWING/EDITING OF THE LIGHT.
                        PANELS::LightPanel::UpdateAndRender(*light);

                        ImGui::TreePop();
                    }

                    // MOVE ONTO THE APPROPRIATE NEXT LIGHT.
                    // This convoluted way of removing lights by tracking all of the additional data
                    // is needed because C++ does not provide:
                    // - A way to erase elements from a vector by index.
                    // - A way to get element indices while iterating over a vector via other means.
                    // - A way to conditional tweak the logic for updating a loop iterator after each iteration.
                    // - A way to otherwise safely remove items from a vector while iterating over it.
                    if (light_removed)
                    {
                        light = scene.Lights.erase(light);
                    }
                    else
                    {
                        ++light;
                    }
                }

                // ALLOW THE USER TO ADD A LIGHT.
                if (ImGui::Button("Add"))
                {
                    scene.Lights.emplace_back(GRAPHICS::SHADING::LIGHTING::Light{});
                }

                ImGui::TreePop();
            }

            // ALLOW THE USER TO VIEW/EDIT OBJECTS.
            std::size_t object_count = scene.Objects.size();
            if (ImGui::TreeNode("Objects", "Objects (%zu)", object_count))
            {
                // LIST ALL OBJECTS.
                std::size_t object_index = 0;
                for (auto object = scene.Objects.begin(); object != scene.Objects.end(); ++object_index)
                {
                    // DISPLAY INFORMATION FOR THE CURRENT OBJECT.
                    bool object_removed = false;
                    std::string object_tree_label = "Object " + std::to_string(object_index);
                    if (ImGui::TreeNode(object_tree_label.c_str()))
                    {
                        // ALLOW THE USER TO REMOVE THE CURRENT OBJECT.
                        object_removed = ImGui::Button("Remove");

                        // ALLOW VIEWING/EDITING OF THE OBJECT.
                        PANELS::ObjectPanel::UpdateAndRender(*object);

                        ImGui::TreePop();
                    }

                    // MOVE ONTO THE APPROPRIATE NEXT OBJECT.
                    // This convoluted way of removing lights by tracking all of the additional data
                    // is needed because C++ does not provide:
                    // - A way to erase elements from a vector by index.
                    // - A way to get element indices while iterating over a vector via other means.
                    // - A way to conditional tweak the logic for updating a loop iterator after each iteration.
                    // - A way to otherwise safely remove items from a vector while iterating over it.
                    if (object_removed)
                    {
                        object = scene.Objects.erase(object);
                    }
                    else
                    {
                        ++object;
                    }
                }

                // ALLOW THE USER TO LOAD OBJECTS FROM FILE.
                if (ImGui::Button("Load"))
                {
                    std::string model_filepath = GetFilepathToOpenFromUser();
                    if (!model_filepath.empty())
                    {
                        std::optional<GRAPHICS::MODELING::Model> current_model = GRAPHICS::MODELING::WavefrontObjectModel::Load(model_filepath);
                        if (current_model)
                        {
                            GRAPHICS::Object3D new_object = GRAPHICS::Object3D();
                            new_object.Model = *current_model;
                            scene.Objects.emplace_back(new_object);
                            /// @todo   Will need to find way to load into graphics device.
                        }
                    }
                }

                // ALLOW THE USER TO ADD ARBITRARY NEW OBJECTS.
                // This button is placed on the same line as the previous button for better use of space.
                ImGui::SameLine();
                if (ImGui::Button("Add"))
                {
                    scene.Objects.emplace_back(GRAPHICS::Object3D{});
                }

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
}
