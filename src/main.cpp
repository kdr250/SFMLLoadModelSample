#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
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

std::vector<Vertex> vertices;

std::vector<unsigned int> indices;

int main()
{
    sf::ContextSettings context;
    context.depthBits = 24;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Window", sf::Style::Default, context);
    window.setActive(true);

    glewInit();
    glewExperimental = GL_TRUE;

    glEnable(GL_TEXTURE_2D);

    sf::Image image;
    if (!image.loadFromFile("resources/texture/viking_room.png"))
    {
        std::cerr << "failed to load texture image" << std::endl;
        return -1;
    }
    image.flipVertically();

    sf::Texture texture;
    if (!texture.loadFromImage(image))
    {
        std::cerr << "failed to load texture" << std::endl;
        return -1;
    }

    sf::Shader shader;
    if (!shader.loadFromFile("resources/shader/vertex.vert", "resources/shader/fragment.frag"))
    {
        std::cerr << "failed to load shaders" << std::endl;
        return -1;
    }
    shader.setUniform("ourTexture", texture);

    sf::Shader::bind(&shader);

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

        // update
        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
                                      time.getElapsedTime().asSeconds() * glm::radians(90.0f),
                                      glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 view =
            glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 proj =
            glm::perspective(glm::radians(45.0f), window.getSize().x / (float)window.getSize().y, 0.1f, 10.0f);

        sf::Glsl::Mat4 sfModel(glm::value_ptr(model));
        sf::Glsl::Mat4 sfView(glm::value_ptr(view));
        sf::Glsl::Mat4 sfProj(glm::value_ptr(proj));
        shader.setUniform("model", sfModel);
        shader.setUniform("view", sfView);
        shader.setUniform("proj", sfProj);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        window.display();
    }
}
