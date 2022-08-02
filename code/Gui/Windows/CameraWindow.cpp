#include <imgui/imgui.h>
#include "Gui/Windows/CameraWindow.h"

namespace GUI::WINDOWS
{
    /// Updates and renders the window, if open.
    /// @param[in,out]  camera - The camera whose settings to display and potentially update in the window.
    void CameraWindow::UpdateAndRender(GRAPHICS::VIEWING::Camera& camera)
    {
        // DON'T RENDER THE WINDOW IF IT IS CLOSED.
        if (!IsOpen)
        {
            return;
        }

        // RENDER THE WINDOW.
        if (ImGui::Begin("Camera", &IsOpen))
        {
            // ALLOW CHANGING THE PROJECTION.
            ImGui::Text("Projection:");
            if (ImGui::RadioButton("Orthographic", GRAPHICS::VIEWING::ProjectionType::ORTHOGRAPHIC == camera.Projection))
            {
                camera.Projection = GRAPHICS::VIEWING::ProjectionType::ORTHOGRAPHIC;
            }
            if (ImGui::RadioButton("Perspective", GRAPHICS::VIEWING::ProjectionType::PERSPECTIVE == camera.Projection))
            {
                camera.Projection = GRAPHICS::VIEWING::ProjectionType::PERSPECTIVE;
            }

            // ALLOW CHANGING OTHER CAMERA SETTINGS.
            // The ranges for many of these values are currently largely arbitrary, so more work is needed to figure out the best ranges.
            constexpr float MIN_SLIDER_FLOAT_VALUE = -2000.0f;
            constexpr float MAX_SLIDER_FLOAT_VALUE = 2000.0f;
            ImGui::SliderFloat3("World Position:", (float*)&camera.WorldPosition, -20.0f, 40.0f);
            ImGui::SliderFloat3("Coordinate Frame Up:", (float*)&camera.CoordinateFrame.Up, -40.0f, 40.0f);
            ImGui::SliderFloat3("Coordinate Frame Right:", (float*)&camera.CoordinateFrame.Right, -40.0f, 40.0f);
            ImGui::SliderFloat3("Coordinate Frame Forward:", (float*)&camera.CoordinateFrame.Forward, -10.0f, 10.0f);
            ImGui::SliderFloat("Near Clip Plane View Distance:", &camera.NearClipPlaneViewDistance, -30.0f, 30.0f);
            ImGui::SliderFloat("Far Clip Plane View Distance:", &camera.FarClipPlaneViewDistance, -2000.0f, 2000.0f);
            ImGui::SliderFloat("Field of View:", &camera.FieldOfView.Value, -360.0f, 360.0f);
            ImGui::SliderFloat("Viewing Plane Focal Length:", &camera.ViewingPlane.FocalLength, -100.0f, 100.0f);
            ImGui::SliderFloat("Viewing Plane Width:", &camera.ViewingPlane.Width, -100.0f, 100.0f);
            ImGui::SliderFloat("Viewing Plane Height:", &camera.ViewingPlane.Height, -100.0f, 100.0f);
        }
        ImGui::End();
    }
}