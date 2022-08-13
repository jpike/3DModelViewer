#include <cmath>
#include <sstream>
#include <Windows.h>
#include <Windowsx.h>
#include <imgui/backends/imgui_impl_win32.h>
#include "Debugging/Timer.h"
#include "Graphics/CpuRendering/CpuGraphicsDevice.h"
#include "Graphics/Geometry/Sphere.h"
#include "Graphics/Modeling/WavefrontObjectModel.h"
#include "Graphics/RenderingSettings.h"
#include "Graphics/Scene.h"
#include "Gui/Gui.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector2.h"
#include "Windowing/Win32Window.h"

// GLOBALS.
// Global to provide access to them within the window procedure.
/// The window for the application.
static std::unique_ptr<WINDOWING::Win32Window> g_window = nullptr;
/// The rendering settings that can be displayed and updated via the GUI.
static GRAPHICS::RenderingSettings g_rendering_settings = {};
/// The camera that can be updated via the GUI.
static GRAPHICS::VIEWING::Camera g_camera = {};

/// True if a mouse button is down; false if not.
static bool g_mouse_down = false;
/// The previous mouse X position, if a mouse button was down, to help with detecting mouse drags.
static int g_previous_mouse_x = 0;
/// The previous mouse Y position, if a mouse button was down, to help with detecting mouse drags.
static int g_previous_mouse_y = 0;

/// True if the scene has changed; used to allow only re-rendering scenes if a scene changes when ray tracing is used for a feasible frame rate.
static bool g_scene_changed = false;

/// The main window callback procedure for processing messages sent to the main application window.
/// @param[in]  window - Handle to the window.
/// @param[in]  message - The message.
/// @param[in]  w_param - Additional information depending on the type of message.
/// @param[in]  l_param - Additional information depending on the type of message.
/// @return     The result of processing the message.
LRESULT CALLBACK MainWindowCallback(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param)
{
    // HAVE THE GUI HAVE PRIORITY FOR INPUT EVENT HANDLING.
    bool event_handled_by_gui = ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param);
    if (event_handled_by_gui)
    {
        return true;
    }

    // Even if the GUI does not directly handle the event, it might be capturing other input that should
    // prevent other changes to the scene.
    ImGuiContext* im_gui_context = ImGui::GetCurrentContext();
    if (im_gui_context)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool gui_capturing_input = (io.WantCaptureMouse || io.WantCaptureKeyboard);
        if (gui_capturing_input)
        {
            g_scene_changed = true;
            return true;
        }
    }

    // PROCESS ANY OTHER IMPORTANT WINDOW MESSAGES.
    LRESULT messageProcessingResult = 0;
    switch (message)
    {
        case WM_CREATE:
            break;
        case WM_SIZE:
            break;
        case WM_DESTROY:
            break;
        case WM_CLOSE:
        {
            PostQuitMessage(EXIT_SUCCESS);
            break;
        }
        case WM_ACTIVATEAPP:
            break;
        case WM_SYSKEYDOWN:
            break;
        case WM_SYSKEYUP:
            break;
        case WM_KEYUP:
            break;
        case WM_KEYDOWN:
        {
            break;
        }
        case WM_LBUTTONDOWN:
        {
            // TRACK THE MOUSE BEING DOWN AT THE CURRENT POSITION.
            g_mouse_down = true;
            g_previous_mouse_x = GET_X_LPARAM(l_param);
            g_previous_mouse_y = GET_Y_LPARAM(l_param);
            break;
        }
        case WM_LBUTTONUP:
        {
            // TRACK THE MOUSE BEING RELEASED.
            g_mouse_down = false;
            break;
        }
        case WM_MOUSEMOVE:
        {
            // ROTATE THE CAMERA IN RESPONSE TO MOUSE DRAGS.
            if (g_mouse_down)
            {
                // TRACK THE SCENE AS HAVING CHANGED BASED ON USER INPUT.
                g_scene_changed = true;

                /// @todo DragDetect? - https://docs.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations

                // GET THE CURRENT MOUSE COORDINATES.
                int mouse_x_position_in_window_pixels = GET_X_LPARAM(l_param);
                int mouse_y_position_in_window_pixels = GET_Y_LPARAM(l_param);

                // COMPUTE THE DRAG DISTANCE.
                int mouse_x_drag_distance_in_pixels = (mouse_x_position_in_window_pixels - g_previous_mouse_x);
                int mouse_y_drag_distance_in_pixels = (mouse_y_position_in_window_pixels - g_previous_mouse_y);

                int max_2d_drag_distance = std::max(mouse_x_drag_distance_in_pixels, mouse_y_drag_distance_in_pixels);

                /// @todo   This z drag amount is somewhat arbitrary and could use more refinement.
                int z_drag_amount = max_2d_drag_distance - (mouse_y_drag_distance_in_pixels - mouse_x_drag_distance_in_pixels);
                MATH::Vector2i mouse_drag(mouse_x_drag_distance_in_pixels, mouse_y_drag_distance_in_pixels);
                int mouse_drag_distance_in_pixels = mouse_drag.Length();
                std::stringstream mouse_drag_debug_text;
                mouse_drag_debug_text
                    << "Mouse drag:"
                    << "\tX =  " << mouse_x_drag_distance_in_pixels
                    << "\tY = " << mouse_y_drag_distance_in_pixels
                    << "\tZ = " << z_drag_amount
                    << "\tDistance = " << mouse_drag_distance_in_pixels
                    << std::endl;
                OutputDebugString(mouse_drag_debug_text.str().c_str());

                // COMPUTE THE CAMERA ROTATION AMOUNT.
                constexpr float X_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL = 5.0f;
                // Note - negation is important for intuitive behavior.
                MATH::Angle<float>::Degrees x_rotation_amount_in_degrees(X_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL * -mouse_x_drag_distance_in_pixels);
                constexpr float Y_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL = 2.0f;
                MATH::Angle<float>::Degrees y_rotation_amount_in_degrees(Y_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL * -mouse_y_drag_distance_in_pixels);

                constexpr float Z_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL = 2.0f;
                /// @todo   Z-rotation could use more refinement.  Maybe just need coordinate system conversion first?
                MATH::Angle<float>::Degrees z_rotation_amount_in_degrees(Z_ROTATION_AMOUNT_IN_DEGREES_PER_PIXEL * z_drag_amount);

                MATH::Angle<float>::Radians x_rotation_amount_in_radians = MATH::Angle<float>::DegreesToRadians(x_rotation_amount_in_degrees);
                MATH::Angle<float>::Radians y_rotation_amount_in_radians = MATH::Angle<float>::DegreesToRadians(y_rotation_amount_in_degrees);
                MATH::Angle<float>::Radians z_rotation_amount_in_radians = MATH::Angle<float>::DegreesToRadians(z_rotation_amount_in_degrees);
                MATH::Vector3<MATH::Angle<float>::Radians> rotation_angles_in_radians(
                    x_rotation_amount_in_radians,
                    y_rotation_amount_in_radians,
                    z_rotation_amount_in_radians);
                
                MATH::Matrix4x4f camera_y_rotation_matrix = MATH::Matrix4x4f::Identity();
                MATH::Matrix4x4f camera_x_rotation_matrix = MATH::Matrix4x4f::Identity();
                MATH::Matrix4x4f camera_z_rotation_matrix = MATH::Matrix4x4f::Identity();

                /// @todo   Z-rotation with too little dragging is distracting.  Probably a better way to dampen this.
                constexpr int Z_THRESHOLD = 30;
                if (std::abs(z_drag_amount) <= Z_THRESHOLD)
                {
                    camera_y_rotation_matrix = MATH::Matrix4x4f::RotateY(x_rotation_amount_in_radians);
                    camera_x_rotation_matrix = MATH::Matrix4x4f::RotateX(y_rotation_amount_in_radians);
                }
                if (std::abs(z_drag_amount) > Z_THRESHOLD * 10)
                {
                    camera_z_rotation_matrix = MATH::Matrix4x4f::RotateZ(z_rotation_amount_in_radians);
                }
                /// @todo   Note - this is bogus, so separate rotation operations are needed.
                //MATH::Matrix4x4f camera_rotation_matrix = MATH::Matrix4x4f::Rotation(rotation_angles_in_radians);

                MATH::Matrix4x4f camera_rotation_matrix = camera_y_rotation_matrix * camera_x_rotation_matrix * camera_z_rotation_matrix;

                // ROTATE THE CAMERA.
                MATH::Vector4f original_camera_position = MATH::Vector4f::HomogeneousPositionVector(g_camera.WorldPosition);
                MATH::Vector4f new_camera_position_homogeneous = camera_rotation_matrix * original_camera_position;
                MATH::Vector3f new_camera_position(
                    new_camera_position_homogeneous.X,
                    new_camera_position_homogeneous.Y,
                    new_camera_position_homogeneous.Z);

                g_camera.WorldPosition = new_camera_position;
                /// @todo   Cleaner way to preserve/recompute camera settings!
                MATH::Vector3f camera_view_direction = MATH::Vector3f(0.0f, 0.0f, 0.0f) - g_camera.WorldPosition;
                MATH::Vector3f normalized_camera_view_direction = MATH::Vector3f::Normalize(camera_view_direction);
                MATH::Vector3f negative_camera_view_direction = MATH::Vector3f::Scale(-1.0f, normalized_camera_view_direction);
                MATH::Vector4f homogeneous_up = MATH::Vector4f::HomogeneousPositionVector(g_camera.CoordinateFrame.Up);
                MATH::Vector4f homogeneous_transformed_up = camera_rotation_matrix * homogeneous_up;
                MATH::Vector3f transformed_up(homogeneous_transformed_up.X, homogeneous_transformed_up.Y, homogeneous_transformed_up.Z);
                g_camera.CoordinateFrame = MATH::CoordinateFrame::FromUpAndForward(transformed_up, negative_camera_view_direction);

                /// @todo   How to preserve prior settings?
                g_camera.Projection = GRAPHICS::VIEWING::ProjectionType::PERSPECTIVE;
                g_camera.NearClipPlaneViewDistance = 1.0f;
                g_camera.FarClipPlaneViewDistance = 1000.0f;

                // STORE THE PREVIOUS MOUSE COORDINATES THAT HAVE BEEN DRAGGED.
                g_previous_mouse_x = mouse_x_position_in_window_pixels;
                g_previous_mouse_y = mouse_y_position_in_window_pixels;
            }

            break;
        }
        case WM_MOUSEWHEEL:
        {
            // TRACK THE SCENE AS HAVING CHANGED BASED ON USER INPUT.
            g_scene_changed = true;

            // COMPUTE HOW MUCH ZOOMING SHOULD OCCUR.
            constexpr float WHEEL_ROTATIONS_PER_ACTION = 120.0f;
            short wheel_rotations_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            float zoom_units = static_cast<float>(wheel_rotations_delta) / WHEEL_ROTATIONS_PER_ACTION;

            // Rotating the mouse wheel forward results in a positive value, but that should be considered as "zooming in".
            // Since the negative z-axis is "forward" (zooming in), the zoom distance must be negated.
            constexpr float Z_AXIS_IN_OPPOSITE_DIRECTION_FROM_WHEEL_ROTATION = -1.0f;
            constexpr float ZOOM_DISTANCE_PER_WHEEL_ROTATION = 1.0f;
            float signed_zoom_distance = Z_AXIS_IN_OPPOSITE_DIRECTION_FROM_WHEEL_ROTATION * ZOOM_DISTANCE_PER_WHEEL_ROTATION * zoom_units;
            MATH::Vector3f zoom_movement_vector = MATH::Vector3f::Scale(signed_zoom_distance, g_camera.CoordinateFrame.Forward);

            // ZOOM IN THE CAMERA.
            g_camera.WorldPosition += zoom_movement_vector;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(window, &paint);
            EndPaint(window, &paint);
            break;
        }
        default:
            messageProcessingResult = DefWindowProcA(window, message, w_param, l_param);
            break;
    }

    return messageProcessingResult;
}

/// The entry point to the application.
/// @param[in]  application_instance - A handle to the current instance of the application.
/// @param[in]  previous_application_instance - Always NULL.
/// @param[in]  command_line_string - The command line parameters for the application.
/// @param[in]  window_show_code - Controls how the window is to be shown.
/// @return     An exit code.  0 for success.
int CALLBACK WinMain(
    HINSTANCE application_instance,
    HINSTANCE previous_application_instance,
    LPSTR command_line_string,
    int window_show_code)
{
    // REFERENCE UNUSED PARAMETERS TO PREVENT COMPILER WARNINGS.
    previous_application_instance;
    command_line_string;
    window_show_code;

    // DEFINE PARAMETERS FOR THE WINDOW TO BE CREATED.
    // The structure is zeroed-out initially since it isn't necessary to set all fields.
    WNDCLASSEX window_class = {};

    window_class.cbSize = sizeof(window_class);

    // Ensure that the window gets re-drawn when the client area is moved or re-sized.
    // A unique device context is allocated for each window in the class which can
    // potentially provide performance improvements by Windows not having to re-obtain
    // or re-allocate a device context.
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    window_class.lpfnWndProc = MainWindowCallback;
    window_class.hInstance = application_instance;
    window_class.lpszClassName = "WindowClass";

    // CREATE THE WINDOW.
    constexpr unsigned int SCREEN_WIDTH_IN_PIXELS = 900;
    constexpr unsigned int SCREEN_HEIGHT_IN_PIXELS = 700;
    g_window = WINDOWING::Win32Window::Create(
        window_class,
        "3D Model Viewer",
        static_cast<int>(SCREEN_WIDTH_IN_PIXELS),
        static_cast<int>(SCREEN_HEIGHT_IN_PIXELS));
    bool window_created = (nullptr != g_window);
    if (!window_created)
    {
        OutputDebugString("Failed to create window.");
        return EXIT_FAILURE;
    }

    // CREATE THE GRAPHICS DEVICE.
    std::unique_ptr<GRAPHICS::HARDWARE::IGraphicsDevice> graphics_device = GRAPHICS::CPU_RENDERING::CpuGraphicsDevice::ConnectTo(
        g_rendering_settings.GraphicsDeviceType,
        *g_window);
    assert(graphics_device);

    std::optional<GUI::Gui> gui = GUI::Gui::Create(*graphics_device, *g_window);
    assert(gui);

    // CREATE A TEST MODEL.
    std::optional<GRAPHICS::Object3D> current_object = GRAPHICS::Object3D();

    std::shared_ptr<GRAPHICS::Material> test_material = std::make_shared<GRAPHICS::Material>();
    test_material->Shading = GRAPHICS::SHADING::ShadingType::MATERIAL;
    test_material->DiffuseProperties.Color = GRAPHICS::Color::WHITE;
    test_material->DiffuseProperties.Texture = GRAPHICS::IMAGES::Bitmap::LoadPng("D:/temp/assets/test_texture.png", GRAPHICS::ColorFormat::RGBA);

    GRAPHICS::Mesh test_mesh;
    test_mesh.Name = "test_mesh";
#if SINGLE_TRIANGLE
    GRAPHICS::Triangle triangle;
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(0.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(-1.0f, -1.0f, 0.0f) ,
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(0.0f, 1.0f),
        },
    };
    triangle.Material = test_material;
    test_mesh.Triangles.push_back(triangle);
#else
    GRAPHICS::GEOMETRY::Triangle triangle;
#if 1
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            //.Color = GRAPHICS::Color::RED,
            .TextureCoordinates = MATH::Vector2f(0.0f, 0.0f), // red
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, -1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            //.Color = GRAPHICS::Color(1.0f, 1.0f, 0.0f, 1.0f),
            //.TextureCoordinates = MATH::Vector2f(1.0f, 1.0f), // this and one below are swapped; yellow
            .TextureCoordinates = MATH::Vector2f(0.0f, 1.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            //.Color = GRAPHICS::Color::BLUE,
            //.TextureCoordinates = MATH::Vector2f(0.0f, 1.0f), // this and one above are swapped; blue
            .TextureCoordinates = MATH::Vector2f(1.0f, 1.0f),
        },
    };
    triangle.Material = test_material;
    test_mesh.Triangles.push_back(triangle);
#endif

#if 1
    triangle.Vertices =
    {
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, -1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(1.0f, 1.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(1.0f, 1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(1.0f, 0.0f),
        },
        GRAPHICS::VertexWithAttributes
        {
            .Position = MATH::Vector3f(0.0f, 1.0f, 0.0f),
            .Color = GRAPHICS::Color::WHITE,
            .TextureCoordinates = MATH::Vector2f(0.0f, 0.0f),
        },
    };
    triangle.Material = test_material;
    test_mesh.Triangles.push_back(triangle);
#endif
#endif

    current_object->Model.MeshesByName["test_mesh"] = test_mesh;

    graphics_device->Load(*current_object);

    // INITIALIZE THE CAMERA.
    g_camera = GRAPHICS::VIEWING::Camera::LookAtFrom(MATH::Vector3f(0.0f, 0.0f, 0.0f), MATH::Vector3f(0.0f, 5.0f, 20.0f));
    g_camera.Projection = GRAPHICS::VIEWING::ProjectionType::PERSPECTIVE;
    g_camera.NearClipPlaneViewDistance = 1.0f;
    g_camera.FarClipPlaneViewDistance = 1000.0f;

    // INITIALIZE THE SCENE.
    GRAPHICS::Scene test_scene;
    test_scene.BackgroundColor = GRAPHICS::Color::BLACK;
    // Some lights are needed for most kinds of rendering.
    test_scene.Lights = std::vector<GRAPHICS::SHADING::LIGHTING::Light>();
    test_scene.Lights.emplace_back(
        GRAPHICS::SHADING::LIGHTING::Light
        {
            .Type = GRAPHICS::SHADING::LIGHTING::LightType::POINT,
            .Color = GRAPHICS::Color(1.0f, 1.0f, 1.0f, 1.0f),
            .PointLightWorldPosition = MATH::Vector3f(0.0f, 0.0f, 5.0f)
        });

    test_scene.Objects.emplace_back(*current_object);

    // ADD SOME SPHERES FOR RAY TRACING.
#if SPHERES
    GRAPHICS::Object3D spheres;
    GRAPHICS::GEOMETRY::Sphere red_sphere;
    red_sphere.CenterPosition = MATH::Vector3f(0.0f, -1.0f, -3.0f);
    red_sphere.Radius = 1.0f;
    red_sphere.Material = std::make_shared<GRAPHICS::Material>();
    red_sphere.Material->DiffuseProperties.Color = GRAPHICS::Color::RED;
    spheres.Spheres.emplace_back(red_sphere);

    GRAPHICS::GEOMETRY::Sphere blue_sphere;
    blue_sphere.CenterPosition = MATH::Vector3f(2.0f, 0.0f, -4.0f);
    blue_sphere.Radius = 1.0f;
    blue_sphere.Material = std::make_shared<GRAPHICS::Material>();
    blue_sphere.Material->DiffuseProperties.Color = GRAPHICS::Color::BLUE;
    spheres.Spheres.emplace_back(blue_sphere);

    GRAPHICS::GEOMETRY::Sphere green_sphere;
    green_sphere.CenterPosition = MATH::Vector3f(-2.0f, 0.0f, -4.0f);
    green_sphere.Radius = 1.0f;
    green_sphere.Material = std::make_shared<GRAPHICS::Material>();
    green_sphere.Material->DiffuseProperties.Color = GRAPHICS::Color::GREEN;
    spheres.Spheres.emplace_back(green_sphere);

    test_scene.Objects.emplace_back(spheres);
#endif

    // RUN A MESSAGE LOOP.
    bool running = true;
    while (running)
    {
        // PROCESS ANY MESSAGES FOR THE APPLICATION WINDOW.
        MSG message;
        auto message_received = [&]()
        {
            const HWND ALL_MESSAGES_FOR_CURRENT_THREAD = NULL;
            const UINT NO_MIN_MESSAGE_RESTRICTION = 0;
            const UINT NO_MAX_MESSAGE_RESTRICTION = 0;

            return PeekMessageA(
                &message,
                ALL_MESSAGES_FOR_CURRENT_THREAD,
                NO_MIN_MESSAGE_RESTRICTION,
                NO_MAX_MESSAGE_RESTRICTION,
                PM_REMOVE);
        };
        while (message_received())
        {
            // STOP RUNNING THE APPLICATION IF THE USER DECIDED TO QUIT.
            if (message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            // TRANSLATE VIRTUAL KEY MESSAGES TO CHARACTER MESSAGES.
            // The return value is not checked since failure is not problematic
            // and the only thing that could be done is logging.
            TranslateMessage(&message);

            // DISPATCH THE MESSAGE TO THE WINDOW PROCEDURE.
            // The return value is simply the return value from the window procedure.
            // Nothing value could be done with it besides logging, so it is ignored.
            DispatchMessage(&message);
        }

        // RENDER THE TEST SCENE.
        // For a more reasonable frame rate when using ray tracing, re-rendering is only done if the scene has changed.
        if (graphics_device->Type() == GRAPHICS::HARDWARE::GraphicsDeviceType::CPU_RAY_TRACER)
        {
            if (g_scene_changed)
            {
                graphics_device->Render(test_scene, g_camera, g_rendering_settings);
            }
        }
        else
        {
            graphics_device->Render(test_scene, g_camera, g_rendering_settings);
        }
        // The scene has no longer changed since last beeing rendered.
        g_scene_changed = false;

        // UPDATE AND RENDER THE GUI.
        GRAPHICS::HARDWARE::GraphicsDeviceType old_graphics_device_type = g_rendering_settings.GraphicsDeviceType;
        gui->UpdateAndRender(*graphics_device, test_scene, g_camera, g_rendering_settings);
        GRAPHICS::HARDWARE::GraphicsDeviceType new_graphics_device_type = g_rendering_settings.GraphicsDeviceType;

        // DISPLAY THE RENDERED FRAME IN THE WINDOW.
        graphics_device->DisplayRenderedImage(*g_window);

        // SWITCH TYPES OF GRAPHICS DEVICES IF APPLICABLE.
        bool graphics_device_type_changed = (old_graphics_device_type != new_graphics_device_type);
        if (graphics_device_type_changed)
        {
            // SHUTDOWN THE OLD GRAPHICS DEVICE.
            graphics_device->Shutdown();

            // CREATE THE NEW TYPE OF GRAPHICS DEVICE.
            graphics_device = GRAPHICS::HARDWARE::IGraphicsDevice::Create(new_graphics_device_type, *g_window);

            // LOAD OBJECTS INTO THE NEW GRAPHICS DEVICE.
            for (GRAPHICS::Object3D& object : test_scene.Objects)
            {
                graphics_device->Load(object);
            }

            // RE-INITIALIZE THE GUI.
            gui = GUI::Gui::Create(*graphics_device, *g_window);

            /// @todo   Better way to preserve camera settings!
            g_camera = GRAPHICS::VIEWING::Camera::LookAtFrom(MATH::Vector3f(0.0f, 0.0f, 0.0f), MATH::Vector3f(0.0f, 5.0f, 20.0f));
            g_camera.Projection = GRAPHICS::VIEWING::ProjectionType::PERSPECTIVE;
            g_camera.NearClipPlaneViewDistance = 1.0f;
            g_camera.FarClipPlaneViewDistance = 1000.0f;
        }

        // LOAD A NEW MODEL IF APPLICABLE.
        if (!gui->SelectedFilepath.empty())
        {
            std::optional<GRAPHICS::MODELING::Model> current_model = GRAPHICS::MODELING::WavefrontObjectModel::Load(gui->SelectedFilepath);

            if (current_model)
            {
                current_object = GRAPHICS::Object3D();
                current_object->Model = *current_model;
                graphics_device->Load(*current_object);

                test_scene.Objects.clear();
                test_scene.Objects.emplace_back(*current_object);
            }            
        }
    }

    // SHUTDOWN SUBSYSTEMS.
    if (gui)
    {
        gui->Shutdown(g_rendering_settings.GraphicsDeviceType);
    }
    if (graphics_device)
    {
        graphics_device->Shutdown();
    }

    return EXIT_SUCCESS;
}

