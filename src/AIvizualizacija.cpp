#include "lib.hpp"
#include <sstream>

struct Button {
    sf::RectangleShape shape;
    sf::Text text;

    Button(float x, float y, float width, float height, const std::string& label, sf::Font& font) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color::Blue);
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(2);

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);
        text.setPosition(x + (width - text.getLocalBounds().width) / 2,
                         y + (height - text.getLocalBounds().height) / 2 - 6);
    }

    bool isClicked(sf::Vector2i mousePos) const {
        return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
        window.draw(text);
    }
};

static float mapX(double x, float offsetX, float width, double minX, double maxX) {
    return offsetX + static_cast<float>((x - minX) * (width / (maxX - minX)));
}

static float mapY(double y, float offsetY, float height, double yScale) {
    float centerY = offsetY + height / 2;
    return centerY - static_cast<float>(y * yScale);
}

static double computeMaxAbsY(double minX, double maxX) {
    const int samples = 200;
    double maxAbs = 0.0;
    for (int i = 0; i <= samples; ++i) {
        double t = static_cast<double>(i) / samples;
        double x = minX + t * (maxX - minX);
        double y = f(x);
        maxAbs = std::max(maxAbs, std::abs(y));
    }
    return maxAbs > 0.0 ? maxAbs : 1.0;
}

static void applyPaddedRange(double baseMinX, double baseMaxX, double rawMinX, double rawMaxX, double minSpan, double padRatio, double& outMinX, double& outMaxX) {
    double span = rawMaxX - rawMinX;
    if (span < minSpan) {
        double center = (rawMinX + rawMaxX) / 2;
        rawMinX = center - minSpan / 2;
        rawMaxX = center + minSpan / 2;
        span = rawMaxX - rawMinX;
    }
    double pad = span * padRatio;
    outMinX = std::max(baseMinX, rawMinX - pad);
    outMaxX = std::min(baseMaxX, rawMaxX + pad);
    if (outMaxX <= outMinX) {
        outMinX = baseMinX;
        outMaxX = baseMaxX;
    }
}

struct IDPStep {
    double l;
    double r;
    double x1;
    double xm;
    double x2;
};

struct GoldenStep {
    double l;
    double r;
    double x1;
    double x2;
};

struct NewtonStep {
    double x;
};

static void collectIDPSteps(double r, double l, std::vector<IDPStep>& steps) {
    double L = r - l;
    double xm = (l + r) / 2;
    double x1, x2;
    int iteration = 0;

    while (L >= eps && iteration < 50) {
        iteration++;
        x1 = l + L / 4;
        x2 = r - L / 4;
        steps.push_back({l, r, x1, xm, x2});

        if (f(x1) < f(xm)) {
            r = xm;
            xm = x1;
        }
        else if (f(x2) < f(xm)) {
            l = xm;
            xm = x2;
        }
        else {
            l = x1;
            r = x2;
        }

        L = r - l;
    }
}

static void collectGoldenSteps(double r, double l, std::vector<GoldenStep>& steps) {
    double fi = (sqrt(5) - 1) / 2;
    double L = r - l;
    double x1 = r - fi * L;
    double x2 = l + fi * L;
    int iteration = 0;

    steps.push_back({l, r, x1, x2});
    while (L >= eps && iteration < 50) {
        iteration++;
        if (f(x2) < f(x1)) {
            l = x1;
            x1 = x2;
            L = r - l;
            x2 = l + fi * L;
        }
        else {
            r = x2;
            x2 = x1;
            L = r - l;
            x1 = r - fi * L;
        }
        steps.push_back({l, r, x1, x2});
    }
}

static void collectNewtonSteps(std::vector<NewtonStep>& steps) {
    double x = 5.0;
    int iteration = 0;
    steps.push_back({x});
    while (iteration < 50) {
        double xi = x - df(x) / ddf(x);
        steps.push_back({xi});
        if (abs(xi - x) < eps) {
            break;
        }
        x = xi;
        iteration++;
    }
}

void openWindow() {
    sf::RenderWindow window(sf::VideoMode(1400, 900), "Algorithm Visualization");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        return;
    }

    const double l = 0.0;
    const double r = 10.0;
    const double baseMinX = std::max(l, -1.0);
    const double baseMaxX = r;
    const double minSpan = 2.0;
    const double padRatio = 0.1;

    std::vector<IDPStep> idpSteps;
    std::vector<GoldenStep> goldenSteps;
    std::vector<NewtonStep> newtonSteps;
    collectIDPSteps(r, l, idpSteps);
    collectGoldenSteps(r, l, goldenSteps);
    collectNewtonSteps(newtonSteps);

    const size_t maxSteps = std::max(idpSteps.size(), std::max(goldenSteps.size(), newtonSteps.size()));
    size_t currentStep = 0;

    const double commonMaxAbsY = std::min(5.0, computeMaxAbsY(baseMinX, baseMaxX));
    const double commonYScale = (350 * 0.45) / commonMaxAbsY;

    Button prevButton(50, 850, 80, 40, "<", font);
    Button nextButton(1320, 850, 80, 40, ">", font);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (nextButton.isClicked(mousePos) && currentStep + 1 < maxSteps) {
                    currentStep++;
                }
                if (prevButton.isClicked(mousePos) && currentStep > 0) {
                    currentStep--;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right && currentStep + 1 < maxSteps) {
                    currentStep++;
                }
                if (event.key.code == sf::Keyboard::Left && currentStep > 0) {
                    currentStep--;
                }
            }
        }

        window.clear(sf::Color::White);

        std::vector<double> idpNow;
        std::vector<double> goldenNow;
        std::vector<double> newtonNow;
        double idpMinX = baseMinX;
        double idpMaxX = baseMaxX;
        double goldenMinX = baseMinX;
        double goldenMaxX = baseMaxX;
        double newtonMinX = baseMinX;
        double newtonMaxX = baseMaxX;
        if (currentStep < idpSteps.size()) {
            double rawMinX = std::max(idpSteps[currentStep].l, -1.0);
            double rawMaxX = idpSteps[currentStep].r;
            applyPaddedRange(baseMinX, baseMaxX, rawMinX, rawMaxX, minSpan, padRatio, idpMinX, idpMaxX);
            idpNow = {idpSteps[currentStep].x1, idpSteps[currentStep].xm, idpSteps[currentStep].x2};
        }
        if (currentStep < goldenSteps.size()) {
            double rawMinX = std::max(goldenSteps[currentStep].l, -1.0);
            double rawMaxX = goldenSteps[currentStep].r;
            applyPaddedRange(baseMinX, baseMaxX, rawMinX, rawMaxX, minSpan, padRatio, goldenMinX, goldenMaxX);
            goldenNow = {goldenSteps[currentStep].x1, goldenSteps[currentStep].x2};
        }
        if (currentStep < newtonSteps.size()) {
            double centerX = newtonSteps[currentStep].x;
            double rawMinX = centerX - minSpan / 2;
            double rawMaxX = centerX + minSpan / 2;
            applyPaddedRange(baseMinX, baseMaxX, rawMinX, rawMaxX, minSpan, padRatio, newtonMinX, newtonMaxX);
            newtonNow = {newtonSteps[currentStep].x};
        }

        // Draw three graphs
        drawGraph(window, 30, 50, 430, 350, baseMinX, baseMaxX, font, idpNow, sf::Color::Magenta, commonYScale);
        drawGraph(window, 480, 50, 430, 350, baseMinX, baseMaxX, font, goldenNow, sf::Color::Blue, commonYScale);
        drawGraph(window, 930, 50, 430, 350, baseMinX, baseMaxX, font, newtonNow, sf::Color::Green, commonYScale);

        sf::Text title;
        title.setFont(font);
        title.setCharacterSize(18);
        title.setFillColor(sf::Color::Black);

        title.setString("IDP");
        title.setPosition(50, 20);
        window.draw(title);

        title.setString("Aukso pjuvis");
        title.setPosition(500, 20);
        window.draw(title);

        title.setString("Niutono metodas");
        title.setPosition(950, 20);
        window.draw(title);

        std::ostringstream idpInfo;
        idpInfo.setf(std::ios::fixed);
        idpInfo << std::setprecision(10);
        if (currentStep < idpSteps.size()) {
            idpInfo << "x1=" << idpSteps[currentStep].x1 << "  f=" << f(idpSteps[currentStep].x1) << "\n";
            idpInfo << "xm=" << idpSteps[currentStep].xm << "  f=" << f(idpSteps[currentStep].xm) << "\n";
            idpInfo << "x2=" << idpSteps[currentStep].x2 << "  f=" << f(idpSteps[currentStep].x2);
        } else {
            idpInfo << "x1=  f=\n" << "xm=  f=\n" << "x2=  f=";
        }
        sf::Text idpText;
        idpText.setFont(font);
        idpText.setCharacterSize(14);
        idpText.setFillColor(sf::Color::Black);
        idpText.setPosition(50, 420);
        idpText.setString(idpInfo.str());
        window.draw(idpText);

        std::ostringstream goldenInfo;
        goldenInfo.setf(std::ios::fixed);
        goldenInfo << std::setprecision(10);
        if (currentStep < goldenSteps.size()) {
            goldenInfo << "x1=" << goldenSteps[currentStep].x1 << "  f=" << f(goldenSteps[currentStep].x1) << "\n";
            goldenInfo << "x2=" << goldenSteps[currentStep].x2 << "  f=" << f(goldenSteps[currentStep].x2);
        } else {
            goldenInfo << "x1=  f=\n" << "x2=  f=";
        }
        sf::Text goldenText;
        goldenText.setFont(font);
        goldenText.setCharacterSize(14);
        goldenText.setFillColor(sf::Color::Black);
        goldenText.setPosition(500, 420);
        goldenText.setString(goldenInfo.str());
        window.draw(goldenText);

        std::ostringstream newtonInfo;
        newtonInfo.setf(std::ios::fixed);
        newtonInfo << std::setprecision(10);
        if (currentStep < newtonSteps.size()) {
            newtonInfo << "x=" << newtonSteps[currentStep].x << "  f=" << f(newtonSteps[currentStep].x);
        } else {
            newtonInfo << "x=  f=";
        }
        sf::Text newtonText;
        newtonText.setFont(font);
        newtonText.setCharacterSize(14);
        newtonText.setFillColor(sf::Color::Black);
        newtonText.setPosition(950, 420);
        newtonText.setString(newtonInfo.str());
        window.draw(newtonText);

        prevButton.draw(window);
        nextButton.draw(window);

        sf::Text stepText;
        stepText.setFont(font);
        stepText.setCharacterSize(16);
        stepText.setFillColor(sf::Color::Black);
        stepText.setPosition(600, 850);
        stepText.setString("Step: " + std::to_string(currentStep + 1) + " / " + std::to_string(maxSteps));
        window.draw(stepText);

        window.display();
    }
}

void drawAxes(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale, const sf::Font& font) {
    float axisY = mapY(0.0, offsetY, height, yScale);
    sf::Vertex xAxis[] = {
        sf::Vertex(sf::Vector2f(offsetX, axisY), sf::Color::Black),
        sf::Vertex(sf::Vector2f(offsetX + width, axisY), sf::Color::Black)
    };
    window.draw(xAxis, 2, sf::Lines);

    float axisX = offsetX;
    if (minX <= 0.0 && maxX >= 0.0) {
        axisX = mapX(0.0, offsetX, width, minX, maxX);
        sf::Vertex yAxis[] = {
            sf::Vertex(sf::Vector2f(axisX, offsetY), sf::Color::Black),
            sf::Vertex(sf::Vector2f(axisX, offsetY + height), sf::Color::Black)
        };
        window.draw(yAxis, 2, sf::Lines);
    }

    const int ticks = 4;
    const double maxAbsY = (height / 2) / yScale;
    for (int i = -ticks; i <= ticks; ++i) {
        double value = (maxAbsY * i) / ticks;
        float y = mapY(value, offsetY, height, yScale);
        if (y < offsetY || y > offsetY + height) {
            continue;
        }
        sf::Vertex tick[] = {
            sf::Vertex(sf::Vector2f(axisX - 4, y), sf::Color::Black),
            sf::Vertex(sf::Vector2f(axisX + 4, y), sf::Color::Black)
        };
        window.draw(tick, 2, sf::Lines);

        std::ostringstream label;
        label.setf(std::ios::fixed);
            label << std::setprecision(2) << value;
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(12);
        text.setFillColor(sf::Color::Black);
        text.setString(label.str());
        text.setPosition(axisX + 6, y - 8);
        window.draw(text);
    }
}

void drawFunction(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale) {
    if (maxX <= minX) {
        return;
    }

    std::vector<sf::Vertex> functionPoints;
    for (int i = 0; i < static_cast<int>(width); ++i) {
        double t = static_cast<double>(i) / (width - 1);
        double x = minX + t * (maxX - minX);
        double y = f(x);
        float screenX = mapX(x, offsetX, width, minX, maxX);
        float screenY = mapY(y, offsetY, height, yScale);
        if (screenY >= offsetY && screenY <= offsetY + height) {
            functionPoints.emplace_back(sf::Vector2f(screenX, screenY), sf::Color::Red);
        }
    }
    if (!functionPoints.empty()) {
        window.draw(functionPoints.data(), functionPoints.size(), sf::LineStrip);
    }
}

void drawPoints(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale, const std::vector<double>& xs, const sf::Color& color) {
    for (double x : xs) {
        if (x < minX || x > maxX) {
            continue;
        }
        double y = f(x);
        float screenX = mapX(x, offsetX, width, minX, maxX);
        float screenY = mapY(y, offsetY, height, yScale);
        if (screenY < offsetY || screenY > offsetY + height) {
            continue;
        }
        sf::CircleShape point(3);
        point.setPosition(screenX - 3, screenY - 3);
        point.setFillColor(color);
        window.draw(point);
    }
}

void drawGraph(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, const sf::Font& font, const std::vector<double>& points, const sf::Color& pointColor, double yScale) {
    if (maxX <= minX) {
        return;
    }

    // Draw border
    sf::RectangleShape border(sf::Vector2f(width, height));
    border.setPosition(offsetX, offsetY);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Black);
    border.setOutlineThickness(1);
    window.draw(border);
    
    // Draw axes
    drawAxes(window, offsetX, offsetY, width, height, minX, maxX, yScale, font);

    // Draw function
    drawFunction(window, offsetX, offsetY, width, height, minX, maxX, yScale);

    // Draw all sampled points
    drawPoints(window, offsetX, offsetY, width, height, minX, maxX, yScale, points, pointColor);
}