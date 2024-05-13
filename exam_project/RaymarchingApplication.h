#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>
#include "IKStructure.h"

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

    void InitializeArm();
    void MoveArm();
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

    // IK values
    Bone m_armRoot;
    glm::vec3 m_targetLocation;

    float m_speed;

    bool m_followTarget;
    bool m_targetReached;

    int m_maxSegmentAmount;
    int m_segmentAmount;
};
