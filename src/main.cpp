#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
};

Vertex vertices[] = {
    // positions          // texture coords
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},    // top right
    {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},   // bottom right
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // bottom left
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}    // top left
};

std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

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
    if (!image.loadFromFile("resources/texture/texture.jpg"))
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

    glm::mat4 transform = glm::mat4(1.0f);
    sf::Glsl::Mat4 sfTransform(glm::value_ptr(transform));
    shader.setUniform("transform", sfTransform);

    sf::Shader::bind(&shader);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        window.display();
    }
}
