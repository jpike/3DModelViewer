#include <string>
#include <imgui/imgui.h>
#include "Gui/Controls/ColorEditor.h"
#include "Gui/Panels/MaterialPanel.h"
#include "Gui/Panels/ObjectPanel.h"

namespace GUI::PANELS
{
    /// Updates and renders the panel.
    /// @param[in,out]  object - The object to display and potentially update in the panel.
    void ObjectPanel::UpdateAndRender(GRAPHICS::Object3D& object)
    {
        // ALLOW THE USER TO EDIT THE WORLD POSITION.
        ImGui::SliderFloat3("Position", (float*)&object.WorldPosition, -50.0f, 50.0f);

        // ALLOW THE USER TO EDIT THE ROTATION.
        ImGui::SliderFloat3("Rotation (radians)", (float*)&object.RotationInRadians, -50.0f, 50.0f);

        // ALLOW THE USER TO EDIT THE SCALE.
        ImGui::SliderFloat3("Scale", (float*)&object.Scale, -50.0f, 50.0f);

        // DISPLAY INFORMATION ABOUT THE MODEL.
        if (ImGui::TreeNode("Model"))
        {
            // RENDER TREE NODES FOR ALL MESHES.
            for (auto& [mesh_name, mesh] : object.Model.MeshesByName)
            {
                // RENDER A TREE FOR THE CURRENT MESH.
                std::size_t triangle_count = mesh.Triangles.size();
                std::string mesh_display_text = mesh_name + " (" + std::to_string(triangle_count) + " triangles)";
                if (ImGui::TreeNode(mesh_display_text.c_str()))
                {
                    // ALLOW THE USER TO CHANGE THE VISIBILITY OF THE MESH.
                    ImGui::Checkbox("Visible?", &mesh.Visible);

                    // RENDER INFORMATION ABOUT ALL TRIANGLES.
                    for (std::size_t triangle_index = 0; triangle_index < mesh.Triangles.size(); ++triangle_index)
                    {
                        // RENDER INFORMATION ABOUT THE CURRENT TRIANGLE.
                        std::string triangle_tree_label = "Triangle " + std::to_string(triangle_index);
                        if (ImGui::TreeNode(triangle_tree_label.c_str()))
                        {
                            GRAPHICS::GEOMETRY::Triangle& triangle = mesh.Triangles.at(triangle_index);

                            // ALLOW VIEWING/EDITING THE MATERIAL.
                            if (triangle.Material)
                            {
                                MaterialPanel::UpdateAndRender(*triangle.Material);
                            }

                            // RENDER INFORMATION ABOUT ALL VERTICES.
                            for (std::size_t vertex_index = 0; vertex_index < triangle.Vertices.size(); ++vertex_index)
                            {
                                std::string vertex_tree_label = "Vertex " + std::to_string(vertex_index);
                                if (ImGui::TreeNode(vertex_tree_label.c_str()))
                                {
                                    // ALLOW EDITING KEY PROPERTIES OF THE VERTEX.
                                    GRAPHICS::VertexWithAttributes& vertex = triangle.Vertices.at(vertex_index);

                                    CONTROLS::ColorEditor::DisplayAndAllowEditing("Color", vertex.Color);

                                    ImGui::InputFloat3("Position", (float*)&vertex.Position);
                                    ImGui::InputFloat2("TextureCoordinates", (float*)&vertex.TextureCoordinates);
                                    ImGui::InputFloat2("Normal", (float*)&vertex.Normal);

                                    // END RENDERING THE TREE FOR THE CURRENT VERTEX.
                                    ImGui::TreePop();
                                }
                            }

                            // END RENDERING THE TREE FOR THE CURRENT TRIANGLE.
                            ImGui::TreePop();
                        }
                    }

                    // ALLOW ADDING NEW TRIANGLES.
                    if (ImGui::Button("Add"))
                    {
                        GRAPHICS::GEOMETRY::Triangle new_triangle;
                        new_triangle.Material = std::make_shared<GRAPHICS::Material>();
                        mesh.Triangles.emplace_back(new_triangle);
                    }

                    // END RENDERING THE TREE FOR THE CURRENT MESH.
                    ImGui::TreePop();
                }
            }

            // ALLOW ADDING NEW MESHES.
            /// @todo   Does this handle null terminators?
            constexpr std::size_t MAX_MESH_NAME_LENGTH_IN_CHARACTERS = 32;
            static char new_mesh_name[MAX_MESH_NAME_LENGTH_IN_CHARACTERS] = {};
            if (ImGui::InputText("New Mesh Name", new_mesh_name, MAX_MESH_NAME_LENGTH_IN_CHARACTERS))
            {
                /// @todo   What should happen here?
            }
            ImGui::SameLine();
            if (ImGui::Button("Add"))
            {
                GRAPHICS::Mesh new_mesh = { .Name = new_mesh_name };
                object.Model.MeshesByName[new_mesh.Name] = new_mesh;
            }

            // END RENDERING THE TREE FOR THE CURRENT MODEL.
            ImGui::TreePop();
        }

        // DISPLAY INFORMATION ABOUT SPHERES.
        std::string sphere_root_tree_label = "Spheres (" + std::to_string(object.Spheres.size()) + ")";
        if (ImGui::TreeNode(sphere_root_tree_label.c_str()))
        {
            // RENDER INFORMATION ABOUT ALL SPHERES.
            for (std::size_t sphere_index = 0; sphere_index < object.Spheres.size(); ++sphere_index)
            {
                // RENDER A TREE FOR THE CURRENT SPHERE.
                std::string sphere_tree_label = "Sphere " + std::to_string(sphere_index);
                if (ImGui::TreeNode(sphere_tree_label.c_str()))
                {
                    // ALLOW EDITING KEY PROPERTIES OF THE SPHERE.
                    GRAPHICS::GEOMETRY::Sphere& sphere = object.Spheres.at(sphere_index);

                    if (sphere.Material)
                    {
                        MaterialPanel::UpdateAndRender(*sphere.Material);
                    }

                    ImGui::InputFloat3("Position", (float*)&sphere.CenterPosition);
                    ImGui::InputFloat("Radius", (float*)&sphere.Radius);

                    // END RENDERING THE TREE FOR THE CURRENT SPHERE.
                    ImGui::TreePop();
                }
            }

            // ALLOW ADDING NEW SPHERES.
            if (ImGui::Button("Add"))
            {
                GRAPHICS::GEOMETRY::Sphere new_sphere;
                new_sphere.Material = std::make_shared<GRAPHICS::Material>();
                object.Spheres.emplace_back(new_sphere);
            }

            // END RENDERING THE TREE FOR ALL SPHERES IN THE OBJECT.
            ImGui::TreePop();
        }
    }
}
