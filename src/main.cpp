#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

namespace std
{
    template<>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
                   ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}  // namespace std

bool isCompiled(GLuint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetShaderInfoLog(shader, 511, nullptr, buffer);
        std::cout << "failed to compile GLSL: " << buffer << std::endl;
        return false;
    }
    return true;
}

bool compileShader(const std::string& fileName, GLenum shaderType, GLuint& outShader)
{
    std::ifstream shaderFile(fileName);
    if (!shaderFile.is_open())
    {
        std::cout << "Shader file not found: " << fileName << std::endl;
        return false;
    }

    std::stringstream sstream;
    sstream << shaderFile.rdbuf();
    std::string contents     = sstream.str();
    const char* contentsChar = contents.c_str();

    outShader = glCreateShader(shaderType);

    glShaderSource(outShader, 1, &contentsChar, nullptr);
    glCompileShader(outShader);

    if (!isCompiled(outShader))
    {
        std::cout << "failed to compile shader: " << fileName << std::endl;
        return false;
    }
    return true;
}

bool isValidProgram(const GLuint& shaderProgram)
{
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetProgramInfoLog(shaderProgram, 511, nullptr, buffer);
        std::cout << "GLSL link status: " << buffer << std::endl;
        return false;
    }
    return true;
}

bool loadShader(const std::string& vertName,
                const std::string& fragName,
                GLuint& shaderProgram,
                GLuint& vertexId,
                GLuint& fragId)
{
    // Compile vertex and pixel shaders
    if (!compileShader(vertName, GL_VERTEX_SHADER, vertexId) || !compileShader(fragName, GL_FRAGMENT_SHADER, fragId))
    {
        return false;
    }

    // Now create a shader program that links together the vertex/frag shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexId);
    glAttachShader(shaderProgram, fragId);
    glLinkProgram(shaderProgram);

    // Verify that the program linked successfully
    if (!isValidProgram(shaderProgram))
    {
        return false;
    }

    return true;
}

bool loadTexture(const std::string& fileName, unsigned int& textureId)
{
    int width, height, channels = 0;
    unsigned char* image = SOIL_load_image(fileName.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
    if (image == nullptr)
    {
        std::cout << "failed to load image: " << fileName << " " << SOIL_last_result() << std::endl;
        return false;
    }

    int format = channels == 4 ? GL_RGBA : GL_RGB;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);

    SOIL_free_image_data(image);

    // Generate mipmap for texture
    glGenerateMipmap(GL_TEXTURE_2D);
    // Enable linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Enable anisotropic filtering, if supported
    if (GLEW_EXT_texture_filter_anisotropic)
    {
        // Get the maximum anisotropy value
        GLfloat largest;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest);
        // Enable it
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest);
    }

    return true;
}

int main()
{
    sf::ContextSettings context;
    context.depthBits = 24;

    sf::Window window(sf::VideoMode(800, 600), "Window", sf::Style::Default, context);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    glewInit();
    glewExperimental = GL_TRUE;

    glEnable(GL_TEXTURE_2D);

    GLuint shaderProgram, vertexId, fragId;
    loadShader("resources/shader/vertex.vert", "resources/shader/fragment.frag", shaderProgram, vertexId, fragId);

    unsigned int textureId;
    loadTexture("resources/texture/viking_room.png", textureId);

    // load model
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "resources/model/viking_room.obj"))
    {
        std::cerr << "failed to load obj" << std::endl;
        return -1;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<Vertex, unsigned int> uniqueVertices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex {};
            vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                               attrib.vertices[3 * index.vertex_index + 1],
                               attrib.vertices[3 * index.vertex_index + 2]};
            vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                               attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.color    = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<unsigned int>(vertices.size());
                vertices.emplace_back(vertex);
            }
            indices.emplace_back(uniqueVertices[vertex]);
        }
    }

    std::cout << "Vertices size : " << vertices.size() << std::endl;
    std::cout << "Indices size  : " << indices.size() << std::endl;

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);

    glViewport(0.f, 0.f, 800.f, 600.f);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    sf::Clock time;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        glUseProgram(shaderProgram);

        // update
        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
                                      time.getElapsedTime().asSeconds() * glm::radians(90.0f),
                                      glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 view =
            glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 proj =
            glm::perspective(glm::radians(45.0f), window.getSize().x / (float)window.getSize().y, 0.1f, 10.0f);

        GLuint modelId = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]);
        GLuint viewId = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]);
        GLuint projId = glGetUniformLocation(shaderProgram, "proj");
        glUniformMatrix4fv(projId, 1, GL_FALSE, &proj[0][0]);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        window.display();
    }
}
