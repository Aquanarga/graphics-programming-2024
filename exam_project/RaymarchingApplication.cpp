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

// glm::rotate(glm::radians(90.0f), glm::vec3(1, 0, 0)) --> 90 degrees around the x axis
RaymarchingApplication::RaymarchingApplication()
    : Application(1024, 1024, "Ray-marching demo")
    , m_renderer(GetDevice())
    , m_armRoot(glm::vec3(0, 0, 0), glm::quat(1.0f, glm::vec3()), nullptr)
    , m_targetLocation(glm::vec3(0, 4, 0))
    , m_speed(.1f)
    , m_followTarget(true)
    , m_targetMoved(true)
{
}

void RaymarchingApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeArm();
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
    MoveArm();
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
    camera->SetViewMatrix(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void RaymarchingApplication::InitializeArm()
{
    // glm::rotate(glm::radians(45.0f), glm::vec3(0, 0, 1)) --> 45 degree rotation on the z axis
    // glm::quat(1.0f, glm::vec3()) --> 0 degrees
    m_armRoot.setChild(
        std::make_shared<Bone>(Bone(glm::vec3(3, 0, 0), glm::quat(1.0f, glm::vec3()),
            std::make_shared<Bone>(Bone(glm::vec3(2.25, 0, 0), glm::quat(1.0f, glm::vec3()),
                std::make_shared<Bone>(Bone(glm::vec3(1.5, 0, 0), glm::quat(1.0f, glm::vec3()), nullptr))
            ))
        ))
    );
}

void RaymarchingApplication::InitializeMaterial()
{
    m_material = CreateRaymarchingMaterial("shaders/arm.glsl");

    // Left empty here, since MoveArm will update them properly on the first frame
    std::vector<glm::vec3> joints = { glm::vec3(0.0f), glm::vec3(0.0f) , glm::vec3(0.0f) , glm::vec3(0.0f) };
    std::vector<glm::mat4> bones = { glm::mat4(1.0f), glm::mat4(1.0f) , glm::mat4(1.0f) };

    // Initialize material uniforms
    m_material->SetUniformValues<const glm::vec3>("Joints", joints);
    m_material->SetUniformValues<const glm::mat4>("Bones", bones);
    m_material->SetUniformValue("JointRadius", .5f);
    m_material->SetUniformValue("JointColor", glm::vec3(0, 0, 1));

    m_material->SetUniformValue("BoneRadius", .25f);
    m_material->SetUniformValue("BoneColor", glm::vec3(0, 1, 0));

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

void RaymarchingApplication::MoveArm()
{
    // Move hand towards target, but don't overshoot the target
    glm::vec3 endpoint = m_armRoot.GetEndPoint();
    glm::vec3 target = m_targetLocation;
    glm::vec3 distance = m_targetLocation - endpoint;
    if (glm::length(distance) > 0.1f)
        target = endpoint + normalize(distance) * m_speed;
    else
        m_targetMoved = false;

    if (m_followTarget && m_targetMoved)
    {
        for (int i = 0; i < 3; i++)
            m_armRoot.RunIK(target);
    }

    glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

    std::vector<glm::vec3> joints;
    std::vector<glm::mat4> bones;
    joints = m_armRoot.getCoordinates(joints, glm::mat4(1.0f), glm::mat4(1.0f));
    
    // Apply view matrix to joints and create bone between them
    for (int i = 0; i < joints.size(); ++i)
    {
        joints[i] = ApplyMatrix(joints[i], viewMatrix);
        if (i != 0)
        {
            glm::vec3 halfVector = (joints[i - 1] - joints[i]) / 2.0f;
            glm::mat4 boneMatrix = glm::translate(glm::mat4(1.0f), joints[i] + halfVector);
            boneMatrix = boneMatrix * glm::toMat4(glm::rotation(glm::normalize(glm::vec3(0, 1, 0)), glm::normalize(halfVector)));
            bones.emplace_back(boneMatrix);
        }
    }

    m_material->SetUniformValues<const glm::mat4>("Bones", bones);
    m_material->SetUniformValues<const glm::vec3>("Joints", joints);
    m_material->SetUniformValue("TargetCenter", ApplyMatrix(m_targetLocation, viewMatrix));
}

void RaymarchingApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    // m_cameraController.DrawGUI(m_imGui);

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        if (ImGui::TreeNodeEx("Target", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for sphere parameters
            glm::vec3 compare = m_targetLocation;
            ImGui::DragFloat3("Point", &m_targetLocation[0], 0.1f);
            if (compare != m_targetLocation)
                m_targetMoved = true;
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("TargetRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("TargetColor"));
            ImGui::Checkbox("FollowTarget", &m_followTarget);

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Joints", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for joint parameters
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("JointRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("JointColor"));

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Bones", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for bone parameters
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("BoneRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("BoneColor"));

            ImGui::TreePop();
        }
    }
    m_imGui.EndFrame();
}