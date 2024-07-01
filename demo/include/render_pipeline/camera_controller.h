#pragma once

// Project includes
#include "render_pipeline/camera.h"
#include "graphics/event_collector.h"

// System includes
#include <string>

// Buttons that control the camera movements
enum class NavigationButtons
{
    Forward = 0,
    Backward,
    Left,
    Right,
    Up,
    Down,
    Shift,
    Count
};

// States that the controller can be in
enum class CameraMode
{
    Arcball = 0,
    FPS
};

// Near/Far plane control modes
enum class ClippingMode
{
    Automatic = 0,
    Manual
};

struct Transform
{
    float4 rotation;
    double3 position;
    float3 angles;
};

class CameraController
{
public:
    CameraController();
    ~CameraController();

    // Initialize the controller
    void initialize(uint32_t width, uint32_t height, float fov, const std::string& projPath);

    // Render the UI
    void render_ui(bool region = true);

    // Process key event
    void process_key_event(uint32_t keyCode, bool state);

    // Process a mouse mouvement
    bool process_mouse_movement(int2 mouse, uint2 windowCenter, float4 screenSize);
    void process_mouse_wheel(int wheel);
    bool process_mouse_button(MouseButton button, bool state);

    // Apply the delta time
    void update(double deltaTime);

    // Get camera
    const Camera& get_camera() const { return m_Camera; }
    Camera& get_camera() { return m_Camera; }

    // Update the camera matrices
    void evaluate_camera_matrices();
    void evaluate_distances();
    void change_camera_mode(CameraMode newMode);
    void setup_play_path();
    void stop_play_path();
    void load_camera_path(const char* pathName);
    void save_camera_path(const char* pathName);
    uint32_t play_frame_index() const;
    bool new_frame();
    double distance_to_earth_center() const;

protected:
    // The camera that the controller is handeling
    Camera m_Camera = Camera();
    
    // Button controls
    bool m_ControllerStates[(uint32_t)NavigationButtons::Count] = { false, false, false, false, false, false, false };

    // Flag that defines if we can interact with the camera
    bool m_ActiveInteraction = false;
    std::string m_PathsDir;

    // Speed
    float m_Speed = 0.0f;

    // Duration
    float m_Duration = 0.0f;

    // Is playing
    bool m_IsPlaying = false;
    bool m_LoopAnimation = true;
    float m_PlayTime = 0.0f;
    uint32_t m_FrameIndex = 0;
    double3 m_SavedPosition = {};
    std::vector<double3> m_PositionSpline;
    std::vector<float4> m_RotationSpline;

    // Control points
    std::vector<Transform> m_ControlPoints = {};

    // Current mode of the controller
    CameraMode m_CurrentMode = CameraMode::Arcball;

    // Mode for the clipping plane distances
    ClippingMode m_ClippingMode = ClippingMode::Automatic;

    // Distance to the planet center
    double2 m_DistanceToPlanetCenter = { 0.0, 0.0 };
};