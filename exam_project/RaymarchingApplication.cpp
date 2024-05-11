#include "RaymarchingApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/shader/Material.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

static glm::vec3 ApplyMatrix(glm::vec3 point, glm::mat4 matrix) {
    return glm::vec3(matrix * glm::vec4(point, 1.0f));
}

RaymarchingApplication::RaymarchingApplication()
    : Application(1024, 1024, "Ray-marching demo")
    , m_renderer(GetDevice())
    , m_targetLocation(glm::vec3(0, 2, -10))
    , m_speed(0.05f)
{
}

void RaymarchingApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeMaterial();
    InitializeRenderer();
}

void RaymarchingApplication::Update()
{
    Application::Update();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Set renderer camera
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();
    m_renderer.SetCurrentCamera(camera);

    // Update the material properties
    m_material->SetUniformValue("ProjMatrix", camera.GetProjectionMatrix());
    m_material->SetUniformValue("InvProjMatrix", glm::inverse(camera.GetProjectionMatrix()));
}

void RaymarchingApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    MoveJoints();
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();
}

void RaymarchingApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void RaymarchingApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void RaymarchingApplication::InitializeMaterial()
{
    glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

    m_material = CreateRaymarchingMaterial("shaders/joints.glsl");

    glm::vec3 jointA = glm::vec3(-3, 0, -10);
    glm::vec3 jointB = glm::vec3(-1, 0, -10);
    glm::vec3 jointC = glm::vec3(1, 0, -10);
    m_hand = glm::vec3(3, 0, -10);

    glm::mat4x3 joints(jointA, jointB, jointC, m_hand);

    // Initialize material uniforms
    m_material->SetUniformValue("Joints", joints);
    m_material->SetUniformValue("JointsRadius", .5f);
    m_material->SetUniformValue("JointsColor", glm::vec3(0, 0, 1));

    m_material->SetUniformValue("TargetCenter", m_targetLocation);
    m_material->SetUniformValue("TargetRadius", .25f);
    m_material->SetUniformValue("TargetColor", glm::vec3(1, 0, 0));
}

void RaymarchingApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(m_material));
}

std::shared_ptr<Material> RaymarchingApplication::CreateRaymarchingMaterial(const char* fragmentShaderPath)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way6
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/sdflibrary.glsl");
    fragmentShaderPaths.push_back("shaders/raymarcher.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    fragmentShaderPaths.push_back("shaders/raymarching.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    
    return material;
}

void RaymarchingApplication::MoveJoints()
{
    //std::cout << m_targetLocation.x << ", " << m_targetLocation.y << ", " << m_targetLocation.z << std::endl;
    glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

    // Move hand towards target, but don't overshoot the target
    glm::vec3 distance = m_targetLocation - m_hand;
    if (glm::length(distance) > 0.1f)
        m_hand += normalize(distance) * m_speed;
    else
        m_hand += distance;

    glm::vec3 jointA = glm::vec3(-3, 0, -10);
    glm::vec3 jointB = glm::vec3(-1, 0, -10);
    glm::vec3 jointC = glm::vec3(1, 0, -10);

    glm::mat4x3 joints(ApplyMatrix(jointA, viewMatrix), ApplyMatrix(jointB, viewMatrix), ApplyMatrix(jointC, viewMatrix), ApplyMatrix(m_hand, viewMatrix));

    m_material->SetUniformValue("Joints", joints);
    m_material->SetUniformValue("TargetCenter", ApplyMatrix(m_targetLocation, viewMatrix));
}

void RaymarchingApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    //m_cameraController.DrawGUI(m_imGui);

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        // Get the camera view matrix and transform the sphere center and the box matrix
        glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Joints", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for joint parameters
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("JointsRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("JointsColor"));

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Target", ImGuiTreeNodeFlags_DefaultOpen))
        {

            // Add controls for sphere parameters
            ImGui::DragFloat3("Point", &m_targetLocation[0], 0.1f);
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("TargetRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("TargetColor"));

            ImGui::TreePop();
        }
    }
    m_imGui.EndFrame();
}