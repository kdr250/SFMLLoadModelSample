#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 768), "Shader Smaple", sf::Style::Titlebar | sf::Style::Close);

    sf::Shader shader;
    if (!shader.loadFromFile("resources/shader/shader.vert", "resources/shader/shader.frag"))
    {
        std::cerr << "failed to load shaders" << std::endl;
        return -1;
    }

    sf::Texture texture;
    texture.create(window.getSize().x, window.getSize().y);

    sf::Sprite sprite;
    sprite.setTexture(texture);

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

        window.draw(sprite, &shader);

        window.display();
    }

    return 0;
}
