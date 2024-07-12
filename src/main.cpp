#include <SFML/Graphics.hpp>
#include <iostream>

constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 768;

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                            "Shader Smaple",
                            sf::Style::Titlebar | sf::Style::Close);

    sf::Shader shader;
    if (!shader.loadFromFile("resources/shader/shader.frag", sf::Shader::Fragment))
    {
        std::cerr << "failed to load shaders" << std::endl;
        return -1;
    }

    sf::VertexArray vertices(sf::TriangleStrip, 3);
    vertices[0].position = sf::Vector2f(WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.2f);
    vertices[0].color    = sf::Color::Red;
    vertices[1].position = sf::Vector2f(WINDOW_WIDTH * 0.8f, WINDOW_HEIGHT * 0.8f);
    vertices[1].color    = sf::Color::Green;
    vertices[2].position = sf::Vector2f(WINDOW_WIDTH * 0.2f, WINDOW_HEIGHT * 0.8f);
    vertices[2].color    = sf::Color::Blue;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();

                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape)
                    {
                        window.close();
                    }
            }
        }

        window.clear();

        window.draw(vertices, &shader);

        window.display();
    }

    return 0;
}
