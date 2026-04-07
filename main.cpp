#include "lib.hpp"
#include <fstream>
#include <sstream>

// Global variables for student ID
int a = 3; // TODO: Set your student ID digits
int b = 7; // TODO: Set your student ID digits

// Forward declarations of algorithms
OptimizationResult gradientDescent(double startX, double startY, double learningRate,
                                   double tolerance, int maxIterations);
OptimizationResult steepestDescent(double startX, double startY, double tolerance,
                                   int maxIterations);
OptimizationResult nelderMead(double startX, double startY, double tolerance,
                              int maxIterations);

void visualizeResults(const vector<OptimizationResult> &allResults);
void savePathsToCsv(const vector<OptimizationResult> &allResults);

int main()
{
    cout << fixed << setprecision(8);
    cout << "=== Optimization Methods Laboratory ===" << endl;
    cout << "Student ID digits: a = " << a << ", b = " << b << endl
         << endl;

    // Define starting points
    double x0 = 0.0, y0 = 0.0;
    double x1 = 1.0, y1 = 1.0;
    double xm = a / 10.0, ym = b / 10.0;

    cout << "Starting points:" << endl;
    cout << "  X0 = (" << x0 << ", " << y0 << ")" << endl;
    cout << "  X1 = (" << x1 << ", " << y1 << ")" << endl;
    cout << "  Xm = (" << xm << ", " << ym << ")" << endl
         << endl;

    // Calculate and display values at starting points
    cout << "=== Function and Gradient Values at Starting Points ===" << endl;

    vector<double> startPoints[] = {{x0, y0}, {x1, y1}, {xm, ym}};
    string startNames[] = {"X0", "X1", "Xm"};

    for (int i = 0; i < 3; i++)
    {
        double x = startPoints[i][0];
        double y = startPoints[i][1];

        cout << startNames[i] << " = (" << x << ", " << y << "):" << endl;
        cout << "  f(x,y) = " << objectiveFunction(x, y) << endl;

        double gradX, gradY;
        computeGradient(x, y, gradX, gradY);
        cout << "  Gradient = (" << gradX << ", " << gradY << ")" << endl;
        cout << "  Gradient magnitude = " << gradientMagnitude(x, y) << endl
             << endl;
    }

    // Store all results
    vector<OptimizationResult> allResults;

    // Run algorithms from each starting point
    cout << "=== Running Optimization Algorithms ===" << endl
         << endl;

    vector<vector<double>> starts = {{x0, y0}, {x1, y1}, {xm, ym}};

    for (auto &start : starts)
    {
        cout << "Starting from (" << start[0] << ", " << start[1] << "):" << endl;

        // Gradient Descent
        OptimizationResult gd = gradientDescent(start[0], start[1], 0.01, 1e-6, 10000);
        cout << "  Gradient Descent:" << endl;
        cout << "    Solution: (" << gd.solution[0] << ", " << gd.solution[1] << ")" << endl;
        cout << "    Min value: " << gd.minValue << endl;
        cout << "    Steps: " << gd.steps << ", Evaluations: " << gd.functionEvaluations << endl;
        allResults.push_back(gd);

        // Steepest Descent
        OptimizationResult sd = steepestDescent(start[0], start[1], 1e-6, 10000);
        cout << "  Steepest Descent:" << endl;
        cout << "    Solution: (" << sd.solution[0] << ", " << sd.solution[1] << ")" << endl;
        cout << "    Min value: " << sd.minValue << endl;
        cout << "    Steps: " << sd.steps << ", Evaluations: " << sd.functionEvaluations << endl;
        allResults.push_back(sd);

        // Nelder-Mead
        OptimizationResult nm = nelderMead(start[0], start[1], 1e-6, 10000);
        cout << "  Nelder-Mead (Simplex):" << endl;
        cout << "    Solution: (" << nm.solution[0] << ", " << nm.solution[1] << ")" << endl;
        cout << "    Min value: " << nm.minValue << endl;
        cout << "    Steps: " << nm.steps << ", Evaluations: " << nm.functionEvaluations << endl;
        allResults.push_back(nm);

        cout << endl;
    }

    savePathsToCsv(allResults);

    // Prompt for visualization
    cout << "=== Visualization ===" << endl;
    cout << "Would you like to visualize the objective function and search paths? (y/n): ";
    char response;
    cin >> response;

    if (response == 'y' || response == 'Y')
    {
        visualizeResults(allResults);
    }
    else
    {
        cout << "Skipping visualization." << endl;
    }

    return 0;
}

void savePathsToCsv(const vector<OptimizationResult> &allResults)
{
    ofstream fullFile("optimization_paths_full.csv");
    ofstream sampledFile("optimization_paths_step100.csv");

    if (!fullFile || !sampledFile)
    {
        cout << "Warning: Could not write CSV output files." << endl;
        return;
    }

    const string header = "run_index,algorithm,start_x,start_y,step,x,y,f_value,valid\n";
    fullFile << header;
    sampledFile << header;

    for (size_t i = 0; i < allResults.size(); i++)
    {
        const OptimizationResult &result = allResults[i];
        for (size_t step = 0; step < result.path.size(); step++)
        {
            const double x = result.path[step][0];
            const double y = result.path[step][1];
            const bool valid = isValidPoint(x, y);
            const double f = objectiveFunction(x, y);

            fullFile << i << ","
                     << result.algorithmName << ","
                     << result.startingX << ","
                     << result.startingY << ","
                     << step << ","
                     << x << ","
                     << y << ","
                     << f << ","
                     << (valid ? 1 : 0) << "\n";

            if (step % 100 == 0 || step + 1 == result.path.size())
            {
                sampledFile << i << ","
                            << result.algorithmName << ","
                            << result.startingX << ","
                            << result.startingY << ","
                            << step << ","
                            << x << ","
                            << y << ","
                            << f << ","
                            << (valid ? 1 : 0) << "\n";
            }
        }
    }

    cout << "Saved optimization path data:" << endl;
    cout << "  optimization_paths_full.csv" << endl;
    cout << "  optimization_paths_step100.csv" << endl;
}

// Visualization function
void visualizeResults(const vector<OptimizationResult> &allResults)
{
    const int WINDOW_SIZE = 800;
    const int PADDING = 50;
    const int PLOT_SIZE = WINDOW_SIZE - 2 * PADDING;

    auto toScreen = [&](double x, double y)
    {
        return sf::Vector2f(
            static_cast<float>(PADDING + x * PLOT_SIZE),
            static_cast<float>(WINDOW_SIZE - PADDING - y * PLOT_SIZE));
    };

    const double fMin = -1.0 / 216.0;
    const double fMax = 0.0;
    auto heatColor = [&](double fval)
    {
        float colorIntensity = static_cast<float>(max(0.0, min(1.0, (fval - fMin) / (fMax - fMin))));
        return sf::Color(
            static_cast<sf::Uint8>(230 - 90 * colorIntensity),
            static_cast<sf::Uint8>(230 - 90 * colorIntensity),
            static_cast<sf::Uint8>(255 - 40 * colorIntensity));
    };

    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Optimization Visualization");
    window.setFramerateLimit(60);

    sf::Font overlayFont;
    bool hasOverlayFont = false;
    vector<string> fontCandidates = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf"};
    for (const auto &fontPath : fontCandidates)
    {
        if (overlayFont.loadFromFile(fontPath))
        {
            hasOverlayFont = true;
            break;
        }
    }

    size_t maxPathLen = 0;
    for (const auto &result : allResults)
    {
        maxPathLen = max(maxPathLen, result.path.size());
    }

    size_t currentStep = maxPathLen > 0 ? maxPathLen - 1 : 0;
    const size_t stepJump = 100;
    size_t selectedRun = 0;

    cout << "\nOpening visualization window..." << endl;
    cout << "Legend:" << endl;
    cout << "  Red = Gradient Descent, Green = Steepest Descent, Blue = Nelder-Mead" << endl;
    cout << "  Hollow square = start point, filled circle = final point" << endl;
    cout << "  Small filled dot = current displayed step position" << endl;
    cout << "  Gray triangle = feasible region (x >= 0, y >= 0, x + y <= 1)" << endl;
    cout << "  Heatmap: lighter means higher f(x,y) (closer to 0), darker means lower f(x,y) (closer to -1/216)" << endl;
    cout << "Controls:" << endl;
    cout << "  Right/Left: move by 100 steps" << endl;
    cout << "  Up/Down: move by 1 step" << endl;
    cout << "  Home/End: jump to first/last step" << endl;
    cout << "  Keys 1-9: choose run for live x,y,z and a,b,c panel" << endl;
    cout << "  HUD with step/status is shown in top-left." << endl;
    cout << "Close the window to exit." << endl;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Right)
                {
                    currentStep = min(currentStep + stepJump, maxPathLen > 0 ? maxPathLen - 1 : 0);
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    currentStep = currentStep > stepJump ? currentStep - stepJump : 0;
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    currentStep = min(currentStep + 1, maxPathLen > 0 ? maxPathLen - 1 : 0);
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    currentStep = currentStep > 0 ? currentStep - 1 : 0;
                }
                else if (event.key.code == sf::Keyboard::Home)
                {
                    currentStep = 0;
                }
                else if (event.key.code == sf::Keyboard::End)
                {
                    currentStep = maxPathLen > 0 ? maxPathLen - 1 : 0;
                }
                else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9)
                {
                    size_t idx = static_cast<size_t>(event.key.code - sf::Keyboard::Num1);
                    if (idx < allResults.size())
                    {
                        selectedRun = idx;
                    }
                }
                else if (event.key.code >= sf::Keyboard::Numpad1 && event.key.code <= sf::Keyboard::Numpad9)
                {
                    size_t idx = static_cast<size_t>(event.key.code - sf::Keyboard::Numpad1);
                    if (idx < allResults.size())
                    {
                        selectedRun = idx;
                    }
                }
            }
        }

        std::ostringstream title;
        title << "Optimization Visualization | Step " << currentStep;
        if (maxPathLen > 0)
        {
            title << "/" << (maxPathLen - 1);
        }
        window.setTitle(title.str());

        window.clear(sf::Color::White);

        // Draw feasible region triangle
        sf::ConvexShape feasibleRegion;
        feasibleRegion.setPointCount(3);
        feasibleRegion.setPoint(0, toScreen(0.0, 0.0));
        feasibleRegion.setPoint(1, toScreen(1.0, 0.0));
        feasibleRegion.setPoint(2, toScreen(0.0, 1.0));
        feasibleRegion.setFillColor(sf::Color(245, 245, 245));
        feasibleRegion.setOutlineColor(sf::Color(170, 170, 170));
        feasibleRegion.setOutlineThickness(1.0f);
        window.draw(feasibleRegion);

        // Draw contour-like background using valid objective values only.
        for (int px = PADDING; px <= WINDOW_SIZE - PADDING; px += 4)
        {
            for (int py = PADDING; py <= WINDOW_SIZE - PADDING; py += 4)
            {
                double x = (px - PADDING) / (double)PLOT_SIZE;
                double y = (WINDOW_SIZE - PADDING - py) / (double)PLOT_SIZE;

                if (isValidPoint(x, y))
                {
                    double fval = objectiveFunction(x, y);
                    sf::Color color = heatColor(fval);
                    sf::CircleShape point(1.6f);
                    point.setFillColor(color);
                    point.setPosition(static_cast<float>(px), static_cast<float>(py));
                    window.draw(point);
                }
            }
        }

        // Draw axes
        sf::RectangleShape xAxis(sf::Vector2f(PLOT_SIZE, 1));
        xAxis.setPosition(PADDING, WINDOW_SIZE - PADDING);
        xAxis.setFillColor(sf::Color::Black);
        window.draw(xAxis);

        sf::RectangleShape yAxis(sf::Vector2f(1, PLOT_SIZE));
        yAxis.setPosition(PADDING, PADDING);
        yAxis.setFillColor(sf::Color::Black);
        window.draw(yAxis);

        // Draw tick marks for 0, 0.5, 1 on both axes.
        vector<double> tickValues = {0.0, 0.5, 1.0};
        for (double t : tickValues)
        {
            sf::Vector2f xTickPos = toScreen(t, 0.0);
            sf::RectangleShape xTick(sf::Vector2f(1.0f, 6.0f));
            xTick.setPosition(xTickPos.x, xTickPos.y - 3.0f);
            xTick.setFillColor(sf::Color::Black);
            window.draw(xTick);

            sf::Vector2f yTickPos = toScreen(0.0, t);
            sf::RectangleShape yTick(sf::Vector2f(6.0f, 1.0f));
            yTick.setPosition(yTickPos.x - 3.0f, yTickPos.y);
            yTick.setFillColor(sf::Color::Black);
            window.draw(yTick);
        }

        // Draw x+y=1 boundary of feasible region.
        sf::Vertex boundary[] = {
            sf::Vertex(toScreen(1.0, 0.0), sf::Color(120, 120, 120)),
            sf::Vertex(toScreen(0.0, 1.0), sf::Color(120, 120, 120))};
        window.draw(boundary, 2, sf::Lines);

        // Draw paths from each algorithm with different colors
        vector<sf::Color> colors = {
            sf::Color::Red,   // Gradient Descent
            sf::Color::Green, // Steepest Descent
            sf::Color::Blue   // Nelder-Mead
        };

        for (size_t i = 0; i < allResults.size(); i++)
        {
            const OptimizationResult &result = allResults[i];
            sf::Color color = colors[i % 3];
            size_t visibleSteps = min(currentStep + 1, result.path.size());

            // Draw path
            for (size_t j = 1; j < visibleSteps; j++)
            {
                double x1 = result.path[j - 1][0];
                double y1 = result.path[j - 1][1];
                double x2 = result.path[j][0];
                double y2 = result.path[j][1];

                if (!isValidPoint(x1, y1) || !isValidPoint(x2, y2))
                {
                    continue;
                }

                sf::Vector2f p1 = toScreen(x1, y1);
                sf::Vector2f p2 = toScreen(x2, y2);

                sf::RectangleShape line(sf::Vector2f(
                    sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)), 1));
                float angle = atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159f;
                line.setRotation(angle);
                line.setPosition(p1);
                line.setFillColor(color);
                window.draw(line);

                // Direction arrow every 25th segment and near the end.
                if ((j % 25 == 0 || j + 1 == visibleSteps) && j > 0)
                {
                    float dx = p2.x - p1.x;
                    float dy = p2.y - p1.y;
                    float segLen = sqrt(dx * dx + dy * dy);
                    if (segLen > 8.0f)
                    {
                        sf::ConvexShape arrowHead;
                        arrowHead.setPointCount(3);
                        arrowHead.setPoint(0, sf::Vector2f(0.0f, 0.0f));
                        arrowHead.setPoint(1, sf::Vector2f(-7.0f, -3.5f));
                        arrowHead.setPoint(2, sf::Vector2f(-7.0f, 3.5f));
                        arrowHead.setFillColor(color);
                        arrowHead.setPosition(p2);
                        arrowHead.setRotation(angle);
                        window.draw(arrowHead);
                    }
                }
            }

            // Draw start point as hollow square.
            if (!result.path.empty() && isValidPoint(result.path.front()[0], result.path.front()[1]))
            {
                sf::RectangleShape startPoint(sf::Vector2f(8.0f, 8.0f));
                startPoint.setFillColor(sf::Color::Transparent);
                startPoint.setOutlineColor(color);
                startPoint.setOutlineThickness(1.5f);
                sf::Vector2f p = toScreen(result.path.front()[0], result.path.front()[1]);
                startPoint.setPosition(p.x - 4.0f, p.y - 4.0f);
                window.draw(startPoint);
            }

            // Draw current displayed position.
            if (visibleSteps > 0)
            {
                const vector<double> &current = result.path[visibleSteps - 1];
                if (isValidPoint(current[0], current[1]))
                {
                    sf::CircleShape currentPoint(6.0f);
                    currentPoint.setFillColor(color);
                    sf::Vector2f p = toScreen(current[0], current[1]);
                    currentPoint.setPosition(p.x - 6.0f, p.y - 6.0f);
                    window.draw(currentPoint);
                }
            }

            // Draw final point
            if (!result.path.empty() && isValidPoint(result.solution[0], result.solution[1]))
            {
                sf::CircleShape finalPoint(5);
                finalPoint.setFillColor(sf::Color::Transparent);
                finalPoint.setOutlineColor(color);
                finalPoint.setOutlineThickness(1.5f);
                sf::Vector2f p = toScreen(result.solution[0], result.solution[1]);
                finalPoint.setPosition(p.x - 5.0f, p.y - 5.0f);
                window.draw(finalPoint);
            }
        }

        if (hasOverlayFont)
        {
            // Axis labels and numeric ticks.
            sf::Text xLabel;
            xLabel.setFont(overlayFont);
            xLabel.setCharacterSize(16);
            xLabel.setFillColor(sf::Color::Black);
            xLabel.setString("x");
            xLabel.setPosition(static_cast<float>(PADDING + PLOT_SIZE + 10), static_cast<float>(WINDOW_SIZE - PADDING - 12));
            window.draw(xLabel);

            sf::Text yLabel;
            yLabel.setFont(overlayFont);
            yLabel.setCharacterSize(16);
            yLabel.setFillColor(sf::Color::Black);
            yLabel.setString("y");
            yLabel.setPosition(static_cast<float>(PADDING - 16), static_cast<float>(PADDING - 24));
            window.draw(yLabel);

            for (double t : tickValues)
            {
                ostringstream tickText;
                tickText << t;

                sf::Text xTickText;
                xTickText.setFont(overlayFont);
                xTickText.setCharacterSize(13);
                xTickText.setFillColor(sf::Color::Black);
                xTickText.setString(tickText.str());
                sf::Vector2f xTickPos = toScreen(t, 0.0);
                xTickText.setPosition(xTickPos.x - 8.0f, xTickPos.y + 8.0f);
                window.draw(xTickText);

                sf::Text yTickText;
                yTickText.setFont(overlayFont);
                yTickText.setCharacterSize(13);
                yTickText.setFillColor(sf::Color::Black);
                yTickText.setString(tickText.str());
                sf::Vector2f yTickPos = toScreen(0.0, t);
                yTickText.setPosition(yTickPos.x - 26.0f, yTickPos.y - 8.0f);
                window.draw(yTickText);
            }

            sf::RectangleShape hudBg(sf::Vector2f(500.0f, 90.0f));
            hudBg.setPosition(10.0f, 10.0f);
            hudBg.setFillColor(sf::Color(255, 255, 255, 210));
            hudBg.setOutlineColor(sf::Color(120, 120, 120));
            hudBg.setOutlineThickness(1.0f);
            window.draw(hudBg);

            // Heatmap legend panel (white/light to dark mapping).
            const float legendX = static_cast<float>(WINDOW_SIZE - PADDING - 165);
            const float legendY = static_cast<float>(PADDING + 18);
            const float legendW = 155.0f;
            const float legendH = 132.0f;
            sf::RectangleShape legendBg(sf::Vector2f(legendW, legendH));
            legendBg.setPosition(legendX, legendY);
            legendBg.setFillColor(sf::Color(255, 255, 255, 220));
            legendBg.setOutlineColor(sf::Color(120, 120, 120));
            legendBg.setOutlineThickness(1.0f);
            window.draw(legendBg);

            const float barX = legendX + 12.0f;
            const float barY = legendY + 44.0f;
            const float barW = 22.0f;
            const float barH = 72.0f;
            for (int k = 0; k < static_cast<int>(barH); k++)
            {
                float t = 1.0f - static_cast<float>(k) / (barH - 1.0f);
                double f = fMin + t * (fMax - fMin);
                sf::RectangleShape stripe(sf::Vector2f(barW, 1.0f));
                stripe.setPosition(barX, barY + k);
                stripe.setFillColor(heatColor(f));
                window.draw(stripe);
            }
            sf::RectangleShape barOutline(sf::Vector2f(barW, barH));
            barOutline.setPosition(barX, barY);
            barOutline.setFillColor(sf::Color::Transparent);
            barOutline.setOutlineColor(sf::Color::Black);
            barOutline.setOutlineThickness(1.0f);
            window.draw(barOutline);

            sf::Text legendTitle;
            legendTitle.setFont(overlayFont);
            legendTitle.setCharacterSize(13);
            legendTitle.setFillColor(sf::Color::Black);
            legendTitle.setString("Heatmap f(x,y)");
            legendTitle.setPosition(legendX + 8.0f, legendY + 8.0f);
            window.draw(legendTitle);

            sf::Text topLabel;
            topLabel.setFont(overlayFont);
            topLabel.setCharacterSize(12);
            topLabel.setFillColor(sf::Color::Black);
            topLabel.setString("Light: 0.00000");
            topLabel.setPosition(barX + barW + 8.0f, barY - 4.0f);
            window.draw(topLabel);

            sf::Text bottomLabel;
            bottomLabel.setFont(overlayFont);
            bottomLabel.setCharacterSize(12);
            bottomLabel.setFillColor(sf::Color::Black);
            bottomLabel.setString("Dark: -0.00463");
            bottomLabel.setPosition(barX + barW + 8.0f, barY + barH - 14.0f);
            window.draw(bottomLabel);

            std::ostringstream hud;
            hud << "Step: " << currentStep;
            if (maxPathLen > 0)
            {
                hud << " / " << (maxPathLen - 1);
            }
            hud << "   (Left/Right: +/-100, Up/Down: +/-1)\n";
            hud << "Red=GD  Green=Steepest  Blue=Nelder-Mead\n";
            hud << "Square=start, Big dot=current, Ring=final";

            sf::Text hudText;
            hudText.setFont(overlayFont);
            hudText.setCharacterSize(15);
            hudText.setFillColor(sf::Color::Black);
            hudText.setPosition(18.0f, 16.0f);
            hudText.setString(hud.str());
            window.draw(hudText);

            // Live 3D interpretation panel for a selected run.
            sf::RectangleShape boxPanelBg(sf::Vector2f(520.0f, 110.0f));
            boxPanelBg.setPosition(10.0f, 106.0f);
            boxPanelBg.setFillColor(sf::Color(255, 255, 255, 210));
            boxPanelBg.setOutlineColor(sf::Color(120, 120, 120));
            boxPanelBg.setOutlineThickness(1.0f);
            window.draw(boxPanelBg);

            selectedRun = min(selectedRun, allResults.size() > 0 ? allResults.size() - 1 : 0);
            const OptimizationResult &tracked = allResults[selectedRun];
            size_t visibleTracked = min(currentStep + 1, tracked.path.size());
            size_t trackedStep = visibleTracked > 0 ? visibleTracked - 1 : 0;

            double xCur = 0.0, yCur = 0.0;
            if (!tracked.path.empty())
            {
                xCur = tracked.path[trackedStep][0];
                yCur = tracked.path[trackedStep][1];
            }
            double zCur = 1.0 - xCur - yCur;

            bool edgeValid = (xCur > 1e-12 && yCur > 1e-12 && zCur > 1e-12);
            double edgeA = 0.0, edgeB = 0.0, edgeC = 0.0;
            if (edgeValid)
            {
                edgeA = sqrt((xCur * zCur) / (2.0 * yCur));
                edgeB = sqrt((xCur * yCur) / (2.0 * zCur));
                edgeC = sqrt((yCur * zCur) / (2.0 * xCur));
            }

            auto fmt5 = [](double v)
            {
                ostringstream oss;
                oss << fixed << setprecision(5) << v;
                return oss.str();
            };

            ostringstream panel;
            panel << "Tracked run: " << (selectedRun + 1) << "/" << allResults.size() << " (keys 1-9)";
            panel << "  " << tracked.algorithmName << "  start=(" << fmt5(tracked.startingX) << ", " << fmt5(tracked.startingY) << ")\n";
            panel << "Current point: x=" << fmt5(xCur) << "  y=" << fmt5(yCur) << "  z=1-x-y=" << fmt5(zCur) << "\n";
            if (edgeValid)
            {
                panel << "Implied box edges: a=" << fmt5(edgeA) << "  b=" << fmt5(edgeB) << "  c=" << fmt5(edgeC);
            }
            else
            {
                panel << "Implied box edges: undefined at boundary/outside (need x,y,z > 0).";
            }

            sf::Text panelText;
            panelText.setFont(overlayFont);
            panelText.setCharacterSize(14);
            panelText.setFillColor(sf::Color::Black);
            panelText.setPosition(18.0f, 112.0f);
            panelText.setString(panel.str());
            window.draw(panelText);
        }

        window.display();
    }
}
