#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

float vertices[] = {
    // positions          // texture coords
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  // top right
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // bottom left
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f   // top left
};

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

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

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
        glDrawArrays(GL_QUADS, 0, 4);
        window.display();
    }
}
