#include "TerrainApplication.h"

// (todo) 01.1: Include the libraries you need

#include <cmath>
#include <iostream>
#include <vector>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>


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
struct Vertex {
    Vertex() : Vertex(Vector3(), Vector2(), Vector3()) {}
    Vertex(Vector3 position, Vector2 texture_coordinates, Vector3 color) : 
        position(position), texture_coordinates(texture_coordinates), color(color), normal(Vector3()) {}
    Vector3 position;
    Vector2 texture_coordinates;
    Vector3 color;
    Vector3 normal;
};

static Vector3 ints_to_rgb(float r, float g, float b) {
    return Vector3(r / 255, g / 255, b / 255);
}

static Vector3 get_color(float z) {
    if (z < -0.15) {
        return ints_to_rgb(0, 117, 162); // Water
    }
    if (z < -0.05) {
        return ints_to_rgb(234, 248, 191); // Sand
    }
    if (z < 0.1) {
        return ints_to_rgb(105, 143, 63); // Grass/Forest
    }
    if (z < 0.3) {
        return ints_to_rgb(88, 75, 83); // Mountain
    }
    return ints_to_rgb(255, 255, 255); // Snow
}

static float my_perlin_noise(float x, float y) {
    float lacunarity = 2.0;
    float gain = 0.5;
    int octaves = 6;
    
    float amplitude = 0.7;
    float x_frequency = 1;
    float y_frequency = 1;

    return stb_perlin_fbm_noise3(x * x_frequency, y * y_frequency, 0.0f, lacunarity, gain, octaves) * amplitude;
}

TerrainApplication::TerrainApplication()
    : Application(1024, 1024, "Terrain demo"), m_gridX(16), m_gridY(16), m_shaderProgram(0)
{
}

void TerrainApplication::Initialize()
{
    Application::Initialize();

    // Build shaders and store in m_shaderProgram
    BuildShaders();

    // (todo) 01.1: Create containers for the vertex position
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // (todo) 01.1: Fill in vertex data
    int column_count = m_gridX + 1;
    int row_count = m_gridY + 1;

    float x_scale = (1.0f / m_gridX);
    float y_scale = (1.0f / m_gridY);

    for (int x = 0; x < column_count; ++x)
    {
        float x_cord = x * x_scale - 0.5f;

        for (int y = 0; y < row_count; ++y) {
            float y_cord = (y) * y_scale - 0.5f;

            float z = my_perlin_noise(x_cord, y_cord);
            vertices.push_back(Vertex(Vector3(x_cord, y_cord, z), Vector2(x, y), get_color(z)));

            // Vertices are created one row at a time, starting from y = 0 (-0.5) going up to m_gridY
            // So for each y, we simply go up 1 column (+y), but for each x, we need to go up by an entire row (x * (m_gridY + 1))
            if (x > 0 && y > 0) {
                float a = y + (x * (m_gridY + 1)); // Current vertex, top right
                float b = a - 1; // One down
                float c = y + (x * (m_gridY + 1)) - row_count - 1; // One left, one down
                float d = a - (row_count); // One left

                indices.push_back(c);
                indices.push_back(d);
                indices.push_back(b);

                indices.push_back(d);
                indices.push_back(a);
                indices.push_back(b);
            }
            // The math worked first try, or in other words, I am smart
        }
    }
    
    int vertex_count = vertices.size();
    for (int x = 0; x < column_count; ++x)
    {
        for (int y = 0; y < row_count; ++y) {
            int index = y + (x * row_count);
            // Realized later I could have relied on x/y, instead of index, oh well
            int up = index + 1;
            int down = index - 1;
            int left = index - row_count;
            int right = index + row_count;

            if (index % row_count == 0) {
                down = index;
            }
            if (index % row_count == row_count - 1) {
                up = index;
            }
            if (index < row_count) {
                left = index;
            }
            if (index > vertex_count - row_count - 1) {
                right = index;
            }

            float delta_x = (vertices[right].position.z - vertices[left].position.z) / (vertices[right].position.x - vertices[left].position.x);
            float delta_y = (vertices[up].position.z - vertices[down].position.z) / (vertices[up].position.y - vertices[down].position.y);

            Vertex& vertex = vertices[index];
            vertex.normal = Vector3(delta_x, delta_y, 1.0f).Normalize();
        }
    }

    // (todo) 01.1: Initialize VAO, and VBO
    vao.Bind();

    vbo.Bind();
    vbo.AllocateData(std::span(vertices));

    VertexAttribute position(Data::Type::Float, 3);
    VertexAttribute texture_coords_att(Data::Type::Float, 2);
    VertexAttribute colors_att(Data::Type::Float, 3);
    VertexAttribute normals_att(Data::Type::Float, 3);

    vao.SetAttribute(0, position, 0, sizeof(Vertex));
    vao.SetAttribute(1, texture_coords_att, position.GetSize(), sizeof(Vertex));
    vao.SetAttribute(2, colors_att, position.GetSize() + texture_coords_att.GetSize(), sizeof(Vertex));
    vao.SetAttribute(3, normals_att, position.GetSize() + texture_coords_att.GetSize() + colors_att.GetSize(), sizeof(Vertex));

    // (todo) 01.5: Initialize EBO
    ebo.Bind();
    ebo.AllocateData<unsigned int>(std::span(indices));

    // (todo) 01.1: Unbind VAO, and VBO
    VertexBufferObject::Unbind();
    VertexArrayObject::Unbind();

    // (todo) 01.5: Unbind EBO
    ElementBufferObject::Unbind();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
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
