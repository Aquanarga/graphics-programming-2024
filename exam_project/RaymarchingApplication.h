#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>

class Material;

class RaymarchingApplication : public Application
{
public:
    RaymarchingApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeMaterial();
    void InitializeRenderer();

    std::shared_ptr<Material> CreateRaymarchingMaterial(const char* fragmentShaderPath);

    void MoveJoints();
    void RenderGUI();

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    // Renderer
    Renderer m_renderer;

    // Materials
    std::shared_ptr<Material> m_material;

    // The hand of the robot arm
    glm::vec3 m_hand;

    // The target the robot arm is moving towards
    glm::vec3 m_targetLocation;

    // The speed the hand is moving towards the target at
    float m_speed;
};
