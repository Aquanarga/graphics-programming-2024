#include "TerrainApplication.h"

// (todo) 01.1: Include the libraries you need

#include <cmath>
#include <iostream>
#include <vector>

// Helper structures. Declared here only for this exercise
struct Vector2
{
    Vector2() : Vector2(0.f, 0.f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct Vector3
{
    Vector3() : Vector3(0.f,0.f,0.f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;

    Vector3 Normalize() const
    {
        float length = std::sqrt(1 + x * x + y * y);
        return Vector3(x / length, y / length, z / length);
    }
};

// (todo) 01.8: Declare an struct with the vertex format



TerrainApplication::TerrainApplication()
    : Application(1024, 1024, "Terrain demo"), m_gridX(4), m_gridY(4), m_shaderProgram(0)
{
}

void TerrainApplication::Initialize()
{
    Application::Initialize();

    // Build shaders and store in m_shaderProgram
    BuildShaders();

    // (todo) 01.1: Create containers for the vertex position
    std::vector<Vector3> vertices;
    std::vector<Vector2> texture_coords;
    std::vector<unsigned int> indices;

    // (todo) 01.1: Fill in vertex data
    float x_scale = (1.0f / m_gridX);
    float y_scale = (1.0f / m_gridY);

    // Initial corner
    vertices.push_back(Vector3(-0.5f, -0.5f, 0.0f));
    texture_coords.push_back(Vector2(0, 0));
    // Initial column
    for (int y = 0; y < m_gridY; ++y) {
        float y_cord = (y + 1) * y_scale - 0.5f;
        vertices.push_back(Vector3(-0.5f, y_cord, 0.0f));
        texture_coords.push_back(Vector2(0, y+1));
    }

    for (int x = 0; x < m_gridX; ++x)
    {
        float x_cord = (x + 1) * x_scale - 0.5f;
        // Bottom of x+1'th column
        vertices.push_back(Vector3(x_cord, -0.5f, 0.0f));
        texture_coords.push_back(Vector2(x+1, 0));
        // Rest of x+1'th column
        for (int y = 0; y < m_gridY; ++y) {
            float y_cord = (y + 1) * y_scale - 0.5f;
            vertices.push_back(Vector3(x_cord, y_cord, 0.0f));
            texture_coords.push_back(Vector2(x+1, y+1));

            // Vertices are created one row at a time, starting from y = 0 (-0.5) going up to m_gridY
            // So for each y, we simply go up 1 column (+y), but for each x, we need to go up by an entire row (x * (m_gridY + 1))
            float a = 0 + y + (x * (m_gridY + 1));
            float b = 1 + y + (x * (m_gridY + 1));
            float c = (m_gridY + 2) + y + (x * (m_gridY + 1));
            float d = (m_gridY + 1) + y + (x * (m_gridY + 1));

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(d);

            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
            // The math worked first try, or in other words, I am smart
        }
    }

    // (todo) 01.1: Initialize VAO, and VBO
    vao.Bind();

    vbo.Bind();
    vbo.AllocateData(vertices.size() * (sizeof(Vector3) + sizeof(Vector2)));

    vbo.UpdateData(std::span(vertices));
    vbo.UpdateData(std::span(texture_coords), sizeof(Vector3) * vertices.size());

    // (todo) 01.5: Initialize EBO
    ebo.Bind();
    ebo.AllocateData<unsigned int>(std::span(indices));


    VertexAttribute position(Data::Type::Float, 3);
    VertexAttribute texture_coordinates(Data::Type::Float, 2);
    vao.SetAttribute(0, position, 0);
    vao.SetAttribute(1, texture_coordinates, sizeof(Vector3) * vertices.size());

    // (todo) 01.1: Unbind VAO, and VBO
    VertexBufferObject::Unbind();
    VertexArrayObject::Unbind();

    // (todo) 01.5: Unbind EBO
    ElementBufferObject::Unbind();
}

void TerrainApplication::Update()
{
    Application::Update();

    UpdateOutputMode();
}

void TerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Set shader to be used
    glUseProgram(m_shaderProgram);

    // (todo) 01.1: Draw the grid
    vao.Bind();
    glDrawElements(GL_TRIANGLES, 2*3*m_gridX*m_gridY, GL_UNSIGNED_INT, 0);

}

void TerrainApplication::Cleanup()
{
    Application::Cleanup();
}


void TerrainApplication::BuildShaders()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aColor;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "uniform mat4 Matrix = mat4(1);\n"
        "out vec2 texCoord;\n"
        "out vec3 color;\n"
        "out vec3 normal;\n"
        "void main()\n"
        "{\n"
        "   texCoord = aTexCoord;\n"
        "   color = aColor;\n"
        "   normal = aNormal;\n"
        "   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "uniform uint Mode = 0u;\n"
        "in vec2 texCoord;\n"
        "in vec3 color;\n"
        "in vec3 normal;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   switch (Mode)\n"
        "   {\n"
        "   default:\n"
        "   case 0u:\n"
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "       break;\n"
        "   case 1u:\n"
        "       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
        "       break;\n"
        "   case 2u:\n"
        "       FragColor = vec4(color, 1.0f);\n"
        "       break;\n"
        "   case 3u:\n"
        "       FragColor = vec4(normalize(normal), 1.0f);\n"
        "       break;\n"
        "   case 4u:\n"
        "       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
        "       break;\n"
        "   }\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_shaderProgram = shaderProgram;
}

void TerrainApplication::UpdateOutputMode()
{
    for (int i = 0; i <= 4; i++)
    {
        if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
        {
            int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
            glUseProgram(m_shaderProgram);
            glUniform1ui(modeLocation, i);
            break;
        }
    }
    if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
    {
        const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
        int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
    }
}
