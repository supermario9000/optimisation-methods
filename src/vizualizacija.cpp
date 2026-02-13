#include "lib.hpp"

void openWindow() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Function Visualization");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        drawAxes(window);
        drawFunction(window);

        window.display();
    }
}

void drawAxes(sf::RenderWindow& window) {
    sf::Vertex xAxis[] = {
        sf::Vertex(sf::Vector2f(0, 300), sf::Color::Black),
        sf::Vertex(sf::Vector2f(800, 300), sf::Color::Black)
    };
    sf::Vertex yAxis[] = {
        sf::Vertex(sf::Vector2f(400, 0), sf::Color::Black),
        sf::Vertex(sf::Vector2f(400, 600), sf::Color::Black)
    };
    window.draw(xAxis, 2, sf::Lines);
    window.draw(yAxis, 2, sf::Lines);
}

void drawFunction(sf::RenderWindow& window) {
    std::vector<sf::Vertex> functionPoints;
    for (int i = 0; i < 800; ++i) {
        double x = (i - 400) / 50.0; // Scale and shift x
        double y = f(x); // Get function value
        int screenY = 300 - static_cast<int>(y * 50); // Scale and shift y
        functionPoints.emplace_back(sf::Vector2f(i, screenY), sf::Color::Red);
    }
    window.draw(functionPoints.data(), functionPoints.size(), sf::LineStrip);
}