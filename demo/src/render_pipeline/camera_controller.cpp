// Project includes
#include "render_pipeline/camera_controller.h"
#include "render_pipeline/constants.h"
#include "imgui/imgui.h"
#include "math/operators.h"
#include "math/cubic_spline.h"
#include "graphics/dx12_backend.h"
#include "render_pipeline/constants.h"

// System includes
#include <algorithm>
#include <fstream>
#include <sstream>

static const char* g_CameraModeNames[] = { "Arc Ball", "FPS" };
static const char* g_ClippingModeNames[] = { "Automatic", "Manual" };

CameraController::CameraController()
{
}

CameraController::~CameraController()
{
}

void CameraController::initialize(uint32_t width, uint32_t height, float fov, const std::string& projPath)
{
    // Camera properties
    m_Camera.fov = fov;
    m_Camera.aspectRatio = width / (float)height;
    m_Camera.position = double3({ 0.0, 0.0, -g_EarthRadius * 40 });
    m_Camera.scaleOffset = { 0.0f, 0.0f, 0.0f };
    m_Camera.angles = { -0.1, -PI / 4.2, 0.0 };
    m_Camera.nearFar = { 0.01f, 100.0f };
    m_Speed = 1.0f;
    m_CurrentMode = CameraMode::Arcball;
    m_ActiveInteraction = false;
    m_PathsDir = projPath + "/paths/";
    
    // Path properties
    m_ControlPoints.resize(2);
    m_Duration = 1.0f;
    m_IsPlaying = false;
    m_PlayTime = 0.0f;

    // Additional properties
    evaluate_distances();
}

void CameraController::render_ui(bool region)
{
    int32_t numControlsPoints = (int32_t)m_ControlPoints.size();
    float totalSize = 340.0f + numControlsPoints * 23.0f;
    ImGui::SetNextWindowSize(ImVec2(550.0f, totalSize));
    ImGui::Begin("Camera controls");

    ImGui::SeparatorText("Base Properties");

    // Base properties
    ImGui::InputDouble3("Position", &m_Camera.position.x);
    ImGui::InputFloat3("Angles", &m_Camera.angles.x);
    ImGui::SliderFloat("FOV", &m_Camera.fov, 0.001, 0.8);
    ImGui::InputFloat3("Scale Offset", &m_Camera.scaleOffset.x);

    // Camera speed
    ImGui::InputFloat2("Near/Far", &m_Camera.nearFar.x);

    ImGui::SeparatorText("Control");
    {
        // Previous mode
        CameraMode newCameraMode = m_CurrentMode;
        const char* currentCameraItem = g_CameraModeNames[(uint32_t)newCameraMode];
        if (ImGui::BeginCombo("Control Mode", currentCameraItem)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < IM_ARRAYSIZE(g_CameraModeNames); n++)
            {
                bool is_selected = (currentCameraItem == g_CameraModeNames[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(g_CameraModeNames[n], is_selected))
                {
                    newCameraMode = (CameraMode)n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
            }
            ImGui::EndCombo();
        }
        if (newCameraMode != m_CurrentMode)
            change_camera_mode(newCameraMode);

        // Previous mode
        const char* currentClippingItem = g_ClippingModeNames[(uint32_t)m_ClippingMode];
        if (ImGui::BeginCombo("Clipping Mode", currentClippingItem)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < IM_ARRAYSIZE(g_ClippingModeNames); n++)
            {
                bool is_selected = (currentClippingItem == g_ClippingModeNames[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(g_ClippingModeNames[n], is_selected))
                {
                    m_ClippingMode = (ClippingMode)n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
            }
            ImGui::EndCombo();
        }

        if (m_ClippingMode == ClippingMode::Manual)
        {
            ImGui::InputFloat2("Near/Far", &m_Camera.nearFar.x);
        }
    }

    // Path
    ImGui::SeparatorText("Path");

    {
        // Num of control points
        ImGui::PushItemWidth(100);
        ImGui::InputInt("Control Points", &numControlsPoints);
        numControlsPoints = std::max(2, numControlsPoints);
        m_ControlPoints.resize(numControlsPoints);
        ImGui::SameLine();
        ImGui::InputFloat("Duration", &m_Duration);
        ImGui::Checkbox("Loop", &m_LoopAnimation);
        ImGui::PopItemWidth();
    }

    // Play the path
    ImGui::SameLine();
    if (ImGui::Button("Play"))
        setup_play_path();
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        stop_play_path();

    {
        static char inoutFile[256] = "loop_demo.csv";
        ImGui::PushItemWidth(160);
        ImGui::InputText("Filename", inoutFile, 256);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Save Path"))
            save_camera_path(inoutFile);

        ImGui::SameLine();
        if (ImGui::Button("Load Path"))
            load_camera_path(inoutFile);
    }


    // Display the control points
    for (int32_t controlPointIdx = 0; controlPointIdx < numControlsPoints; ++controlPointIdx)
    {
        Transform& currentTransform = m_ControlPoints[controlPointIdx];

        ImGui::PushItemWidth(175.0f);
        ImGui::InputScalarN("Rot", ImGuiDataType_Float, &currentTransform.rotation.x, 4);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::PushItemWidth(125.0f);
        ImGui::InputScalarN("Pos", ImGuiDataType_Double, &currentTransform.position.x, 3);
        ImGui::PopItemWidth();

        std::string label = "Copy ";
        label += std::to_string(controlPointIdx);
        ImGui::SameLine();
        if (ImGui::Button(label.c_str()))
        {
            currentTransform.position = m_Camera.position;
            float4 quat = matrix_to_quaternion(m_Camera.view);
            currentTransform.rotation = quat;
            currentTransform.angles = m_Camera.angles;
        }

        label = "Apply ";
        label += std::to_string(controlPointIdx);
        ImGui::SameLine();
        if (ImGui::Button(label.c_str()))
        {
            m_Camera.position = currentTransform.position;
            m_Camera.angles = currentTransform.angles;
        }
    }

    ImGui::End();
}

void CameraController::process_key_event(uint32_t keyCode, bool state)
{
    switch (keyCode)
    {
        case 0x44: // D
            m_ControllerStates[(uint32_t)NavigationButtons::Right] = state;
            break;
        case 0x51: // Q
            m_ControllerStates[(uint32_t)NavigationButtons::Left] = state;
            break;
        case 0x5A: // Z
            m_ControllerStates[(uint32_t)NavigationButtons::Forward] = state;
            break;
        case 0x53: // S
            m_ControllerStates[(uint32_t)NavigationButtons::Backward] = state;
            break;
        case 0x41: // A
            m_ControllerStates[(uint32_t)NavigationButtons::Up] = state;
            break;
        case 0x45: // E
            m_ControllerStates[(uint32_t)NavigationButtons::Down] = state;
            break;
        case 0x10: // Shift
            m_ControllerStates[(uint32_t)NavigationButtons::Shift] = state;
            break;
    }
}

bool CameraController:: process_mouse_button(MouseButton button, bool state)
{
    if (button == MouseButton::Right)
    {
        if (m_ActiveInteraction)
            d3d12::window::set_cursor_visibility(true);
        else
            d3d12::window::set_cursor_visibility(false);
        m_ActiveInteraction = !m_ActiveInteraction;
        return true;
    }
    return false;
}

bool CameraController::process_mouse_movement(int2 mouse, uint2 windowCenter, float4 screenSize)
{
    if (m_ActiveInteraction)
    {
        int2 mouv = int2({ mouse.x - (int)windowCenter.x, mouse.y - (int)windowCenter.y });
        m_Camera.angles.x -= mouv.x / screenSize.x * 5.0f;
        m_Camera.angles.y -= mouv.y / screenSize.y * 5.0f;
        return true;
    }
    return false;
}


void CameraController::update(double deltaTime)
{
    if (!m_IsPlaying)
    {
        if (m_ActiveInteraction)
        {
            switch (m_CurrentMode)
            {
            case CameraMode::FPS:
            {
                double3 forwardDir = double3({ m_Camera.view.m[2], m_Camera.view.m[6], m_Camera.view.m[10] });
                double3 rightDir = double3({ m_Camera.view.m[0], m_Camera.view.m[4], m_Camera.view.m[8] });
                double3 upDir = double3({ m_Camera.view.m[1], m_Camera.view.m[5], m_Camera.view.m[9] });
                double speed = m_Speed * deltaTime;

                double3 displacement = double3({ 0.0, 0.0, 0.0 });
                if (m_ControllerStates[(uint32_t)NavigationButtons::Forward])
                {
                    displacement.x += forwardDir.x * speed;
                    displacement.y += forwardDir.y * speed;
                    displacement.z += forwardDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Backward])
                {
                    displacement.x -= forwardDir.x * speed;
                    displacement.y -= forwardDir.y * speed;
                    displacement.z -= forwardDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Left])
                {
                    displacement.x -= rightDir.x * speed;
                    displacement.y -= rightDir.y * speed;
                    displacement.z -= rightDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Right])
                {
                    displacement.x += rightDir.x * speed;
                    displacement.y += rightDir.y * speed;
                    displacement.z += rightDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Up])
                {
                    displacement.x -= upDir.x * speed;
                    displacement.y -= upDir.y * speed;
                    displacement.z -= upDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Down])
                {
                    displacement.x += upDir.x * speed;
                    displacement.y += upDir.y * speed;
                    displacement.z += upDir.z * speed;
                }

                if (m_ControllerStates[(uint32_t)NavigationButtons::Shift])
                {
                    double3 candidatePosition = m_Camera.position + displacement;
                    double radius = length(candidatePosition);
                    m_Camera.position = candidatePosition * m_DistanceToPlanetCenter.x / radius;
                }
                else
                {
                    m_Camera.position = m_Camera.position + displacement;
                }
            }
            break;
            }
        }
    }
    else
    {
        // Done with playing
        if (m_PlayTime >= m_Duration)
        {
            if (m_LoopAnimation)
            {
                // Mark that we are playing
                m_IsPlaying = true;

                // Reset the current play time
                m_PlayTime = 0.0;
                m_FrameIndex = 0;
            }
            else
            {
                m_IsPlaying = false;
                m_Camera.position = m_SavedPosition;
            }
        }
        else
        {
            // Evaluate the spline
            double3 pos = evaluate_catmull_rom_spline<double3, double>(m_PositionSpline, m_PlayTime / m_Duration, m_LoopAnimation);
            float4 rot = evaluate_catmull_rom_spline<float4, float>(m_RotationSpline, m_PlayTime / m_Duration, m_LoopAnimation);
            rot = normalize(rot);

            m_Camera.position = pos;
            m_Camera.view = quaternion_to_matrix(rot);

            // Add the time
            m_PlayTime += (float)deltaTime;
        }
    }
    
    // update the distances
    evaluate_distances();

    // Position has been updated, update the matrices
    evaluate_camera_matrices();
}

void CameraController::evaluate_camera_matrices()
{
    double nearP, farP;
    if (m_ClippingMode == ClippingMode::Automatic)
    {
        // Elevation
        double earthElevation = m_DistanceToPlanetCenter.x - g_EarthRadius;
        double moonElevation = m_DistanceToPlanetCenter.y - g_MoonRadius;

        if (earthElevation < 2000.0 || moonElevation < 5000.0f)
        {
            nearP = 0.1f;
            farP = 200000.0f;
        }
        else
        {
            double minElevation = std::min(earthElevation, moonElevation);
            nearP = minElevation / 50.0;
            double earthFar = std::max(sqrt(m_DistanceToPlanetCenter.x * m_DistanceToPlanetCenter.x - g_EarthRadius * g_EarthRadius) * 2.0, 100.0);
            double moonFar = std::max(sqrt(m_DistanceToPlanetCenter.y * m_DistanceToPlanetCenter.y - g_MoonRadius * g_MoonRadius) * 2.0, 100.0);
            farP = std::min(earthFar, moonFar);
        }

        // Evaluate the near and far distance
        m_Camera.nearFar = { (float)nearP, (float)farP };
    }
    else
    {
        nearP = m_Camera.nearFar.x;
        farP = m_Camera.nearFar.y;
    }

    // Evaluate the projection matrix (normal)
    m_Camera.projection = projection_matrix(m_Camera.fov, (float)nearP, (float)farP, m_Camera.aspectRatio);
    float realScale = exp2f(m_Camera.scaleOffset.x);
    float4x4 zoomMatrix = zoom_matrix(float2({realScale, realScale }), { m_Camera.scaleOffset.y * (realScale - 1), m_Camera.scaleOffset.z * (realScale - 1) });
    m_Camera.projection = mul(zoomMatrix, m_Camera.projection);

    m_Camera.projectionDB = projection_matrix((double)m_Camera.fov, nearP, farP, (double)m_Camera.aspectRatio);
    double realScaleDB = exp2(m_Camera.scaleOffset.x);
    double4x4 zoomMatrixDB = zoom_matrix(double2({ realScaleDB, realScaleDB }), { m_Camera.scaleOffset.y * (realScaleDB - 1), m_Camera.scaleOffset.z * (realScaleDB - 1) });
    m_Camera.projectionDB = mul(zoomMatrixDB, m_Camera.projectionDB);

    if (!m_IsPlaying)
    {
        switch (m_CurrentMode)
        {
        case CameraMode::FPS:
        {
            // Update the view matrix
            const float4x4 rotation_z = rotation_matrix_z(m_Camera.angles.z);
            const float4x4 rotation_y = rotation_matrix_y(m_Camera.angles.x);
            const float4x4 rotation_x = rotation_matrix_x(m_Camera.angles.y);
            m_Camera.view = mul(rotation_z, mul(rotation_x, rotation_y));
        }
        break;
        case CameraMode::Arcball:
        {
            // Update the view matrix
            const float4x4 rotation_z = rotation_matrix_z(m_Camera.angles.z);
            const float4x4 rotation_y = rotation_matrix_y(m_Camera.angles.x);
            const float4x4 rotation_x = rotation_matrix_x(m_Camera.angles.y);
            m_Camera.view = mul(rotation_x, rotation_y);
            double radius = length(m_Camera.position);
            float3 posF = mul(m_Camera.view, float3({ 0.0, 0.0, (float)-radius }));
            m_Camera.position = double3({ posF.x, posF.y, posF.z });
        }
        break;
        }
    }

    // Update the compound matrices
    m_Camera.viewProjection = mul(m_Camera.projection, m_Camera.view);
    m_Camera.viewProjectionDB = mul(m_Camera.projectionDB, convert_to_double(m_Camera.view));

    // Compute the inverse matrices
    m_Camera.invViewProjection = inverse(m_Camera.viewProjection);
}

void CameraController::process_mouse_wheel(int wheel)
{
    if (m_ActiveInteraction)
    {
        switch (m_CurrentMode)
        {
            case CameraMode::FPS:
            {
                if (wheel > 0)
                    m_Speed *= 2.0;
                else
                    m_Speed /= 2.0;
            }
            break;
            case CameraMode::Arcball:
            {
                double elevation = m_DistanceToPlanetCenter.x - g_EarthRadius;
                if (wheel > 0.0)
                    elevation *= 0.75;
                else
                    elevation *= 1.5;
                double newRadius = g_EarthRadius + elevation;
                m_Camera.position = m_Camera.position * newRadius / m_DistanceToPlanetCenter.x;
                evaluate_distances();
            }
            break;
        }            
    }
}

void CameraController::setup_play_path()
{
    // Mark that we are playing
    m_IsPlaying = true;
    
    // Reset the current play time
    m_PlayTime = 0.0;
    m_FrameIndex = 0;

    // Keep track of the previous position
    m_SavedPosition = m_Camera.position;

    // Allocate the buffers at the right sizes
    uint32_t numControlPoints = (uint32_t)m_ControlPoints.size();
    m_PositionSpline.resize(numControlPoints);
    m_RotationSpline.resize(numControlPoints);

    // Save the points
    for (uint32_t ptIdx = 0; ptIdx < numControlPoints; ++ptIdx)
    {
        m_PositionSpline[ptIdx] = m_ControlPoints[ptIdx].position;
        m_RotationSpline[ptIdx] = m_ControlPoints[ptIdx].rotation;

        // Sanitize the rotation
        if (ptIdx > 0)
        {
            float l0 = length(m_RotationSpline[ptIdx] - m_RotationSpline[ptIdx - 1]);
            float l1 = length(m_RotationSpline[ptIdx] + m_RotationSpline[ptIdx - 1]);
            if (l1 < l0)
                m_RotationSpline[ptIdx] = negate(m_RotationSpline[ptIdx]);
        }
    }
}

void CameraController::stop_play_path()
{
    m_IsPlaying = false;
    m_Camera.position = m_SavedPosition;
}

void CameraController::save_camera_path(const char* pathName)
{
    std::ofstream pathFile;
    pathFile.open(m_PathsDir + pathName);
    pathFile << m_ControlPoints.size() << std::endl;
    pathFile << m_Duration << std::endl;
    for (uint32_t ptIdx = 0; ptIdx < m_ControlPoints.size(); ++ptIdx)
    {
        const Transform& cT = m_ControlPoints[ptIdx];
        pathFile << cT.rotation.x <<";" << cT.rotation.y << ";" << cT.rotation.z << ";" << cT.rotation.w
            << ";" << cT.position.x << ";" << cT.position.y << ";" << cT.position.z
            << ";" << cT.angles.x << ";" << cT.angles.y << ";" << cT.angles.z << ";" << std::endl;
    }
    pathFile.close();
}

void CameraController::load_camera_path(const char* pathName)
{
    // Open the file
    std::ifstream pathFile;
    pathFile.open(m_PathsDir + pathName);
    if (!pathFile.is_open())
        return;

    // Read the number of control points
    uint32_t numPoints;
    pathFile >> numPoints;
    m_ControlPoints.resize(numPoints);

    // Read the duration
    pathFile >> m_Duration;

    // Read the individual points
    std::string line;
    for (uint32_t ptIdx = 0; ptIdx < numPoints; ++ptIdx)
    {
        // Read the line
        pathFile >> line;

        // Decompose it
        std::istringstream iss(line);
        Transform& cT = m_ControlPoints[ptIdx];
        char s;
        iss >> cT.rotation.x >> s >> cT.rotation.y >> s >> cT.rotation.z >> s >> cT.rotation.w 
            >> s >> cT.position.x >> s >> cT.position.y >> s >> cT.position.z 
            >> s >> cT.angles.x >> s >> cT.angles.y >> s >> cT.angles.z;
    }
    pathFile.close();
}

uint32_t CameraController::play_frame_index() const
{
    return m_FrameIndex / 2;
}

bool CameraController::new_frame()
{
    if (m_IsPlaying)
    {
        m_FrameIndex++;
        if (m_FrameIndex == 1)
            return true;
        return false;
    }
    return false;
}

void CameraController::change_camera_mode(CameraMode newMode)
{
    switch (newMode)
    {
        case CameraMode::Arcball:
        {
            // Reset the local camera properties
            m_Camera.position = double3({0.0, 0.0, -g_EarthRadius * 40});
            m_Camera.angles = { -0.1, -PI / 4.2, 0.0 };
        }
        break;
        case CameraMode::FPS:
        {
            // Reset the speed
            m_Speed = 1.0f;

            // Compute the 
            float3 posF = mul(m_Camera.view, float3({ 0.0, 0.0, -(g_EarthRadius + 100.0f) }));
            m_Camera.position = double3({ posF.x, posF.y, posF.z });
        }
        break;
    }

    // Update the distances
    evaluate_distances();

    // Set the new mode
    m_CurrentMode = newMode;
}

void CameraController::evaluate_distances()
{
    m_DistanceToPlanetCenter.x = length(m_Camera.position - g_EarthCenter);
    m_DistanceToPlanetCenter.y = length(m_Camera.position - g_MoonCenter);
}

double CameraController::distance_to_earth_center() const
{
    return length(m_Camera.position - g_EarthCenter);
}