#include "lib.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

int a = 3;
int b = 7;
int c = 1;

void savePathsToCsv(const vector<OptimizationResult> &allResults)
{
    ofstream fullFile("optimization_paths_full.csv");
    ofstream sampledFile("optimization_paths_step100.csv");

    if (!fullFile || !sampledFile)
    {
        cout << "Warning: Could not write CSV output files." << endl;
        return;
    }

    const string header = "run_index,penalty_r,algorithm,start_x,start_y,start_z,step,x,y,z,penalty_value,feasible\n";
    fullFile << header;
    sampledFile << header;

    for (size_t i = 0; i < allResults.size(); i++)
    {
        const OptimizationResult &result = allResults[i];
        for (size_t step = 0; step < result.path.size(); step++)
        {
            const double x = result.path[step][0];
            const double y = result.path[step][1];
            const double z = result.path[step][2];
            const Point3D point = {x, y, z};
            const bool feasible = isFeasiblePoint(point);
            const double penaltyVal = penaltyFunction(point, result.penaltyParameter);

            fullFile << i << ","
                     << result.penaltyParameter << ","
                     << result.algorithmName << ","
                     << result.startingPoint[0] << ","
                     << result.startingPoint[1] << ","
                     << result.startingPoint[2] << ","
                     << step << ","
                     << x << ","
                     << y << ","
                     << z << ","
                     << penaltyVal << ","
                     << (feasible ? 1 : 0) << "\n";

            if (step % 100 == 0 || step + 1 == result.path.size())
            {
                sampledFile << i << ","
                            << result.penaltyParameter << ","
                            << result.algorithmName << ","
                            << result.startingPoint[0] << ","
                            << result.startingPoint[1] << ","
                            << result.startingPoint[2] << ","
                            << step << ","
                            << x << ","
                            << y << ","
                            << z << ","
                            << penaltyVal << ","
                            << (feasible ? 1 : 0) << "\n";
            }
        }
    }

    cout << "Issaugoti optimizavimo keliai:" << endl;
    cout << "  optimization_paths_full.csv" << endl;
    cout << "  optimization_paths_step100.csv" << endl;
}

void visualizeResults(const vector<OptimizationResult> &allResults)
{
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 1000;
    const int PADDING = 60;
    const int PLOT_SIZE = WINDOW_HEIGHT - 2 * PADDING;
    const int SIDEBAR_X = PADDING + PLOT_SIZE + 20;
    const int SIDEBAR_W = WINDOW_WIDTH - SIDEBAR_X - PADDING;

    auto toScreen = [&](double x, double y)
    {
        return sf::Vector2f(
            static_cast<float>(PADDING + x * PLOT_SIZE),
            static_cast<float>(WINDOW_HEIGHT - PADDING - y * PLOT_SIZE));
    };

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Dezes optimizavimas: 3D baudos metodas");
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

    size_t currentStep = 0;
    const size_t stepJump = 100; // Up/Down jump
    size_t selectedRun = 0;
    bool rotateMode = false;
    double yaw = 0.65;
    double pitch = 0.45;

    auto hsvToRgb = [](double h, double s, double v)
    {
        h = fmod(fmod(h, 1.0) + 1.0, 1.0);
        s = std::max(0.0, std::min(1.0, s));
        v = std::max(0.0, std::min(1.0, v));

        double c = v * s;
        double hp = h * 6.0;
        double x = c * (1.0 - fabs(fmod(hp, 2.0) - 1.0));
        double r1 = 0.0, g1 = 0.0, b1 = 0.0;

        if (hp < 1.0)
            r1 = c, g1 = x, b1 = 0.0;
        else if (hp < 2.0)
            r1 = x, g1 = c, b1 = 0.0;
        else if (hp < 3.0)
            r1 = 0.0, g1 = c, b1 = x;
        else if (hp < 4.0)
            r1 = 0.0, g1 = x, b1 = c;
        else if (hp < 5.0)
            r1 = x, g1 = 0.0, b1 = c;
        else
            r1 = c, g1 = 0.0, b1 = x;

        double m = v - c;
        return sf::Color(
            static_cast<sf::Uint8>((r1 + m) * 255.0),
            static_cast<sf::Uint8>((g1 + m) * 255.0),
            static_cast<sf::Uint8>((b1 + m) * 255.0));
    };

    vector<sf::Color> runColors;
    runColors.reserve(allResults.size());
    const size_t colorCount = std::max<size_t>(allResults.size(), 1);
    for (size_t i = 0; i < allResults.size(); ++i)
    {
        double hue = static_cast<double>(i) / static_cast<double>(colorCount);
        runColors.push_back(hsvToRgb(hue, 0.85, 0.95));
    }

    cout << "\nAtidaroma 2D projekcija (x-y plokstuma)..." << endl;
    cout << "Valdymas:" << endl;
    cout << "  Left/Right: +1/-1 zingsnis" << endl;
    cout << "  Up/Down: +100/-100 zingsniu" << endl;
    cout << "  Home/End: i pirma/paskutini zingsni" << endl;
    cout << "  1-9: pasirinkti bandyma informaciniam langui" << endl;
    cout << "  [ / ]: perjungti pasirinkta bandyma" << endl;
    cout << "  R: ijungti/isjungti sukima; A/D ir W/S sukti vaizda" << endl;
    cout << "Uzdarykite langa, kad baigtumete." << endl;

    const Point3D analyticalSol = analyticalSolution();

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
                    currentStep = min(currentStep + 1, maxPathLen > 0 ? maxPathLen - 1 : 0);
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    currentStep = currentStep > 0 ? currentStep - 1 : 0;
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    currentStep = min(currentStep + stepJump, maxPathLen > 0 ? maxPathLen - 1 : 0);
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    currentStep = currentStep > stepJump ? currentStep - stepJump : 0;
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
                else if (event.key.code == sf::Keyboard::LBracket)
                {
                    if (!allResults.empty())
                    {
                        selectedRun = (selectedRun + allResults.size() - 1) % allResults.size();
                    }
                }
                else if (event.key.code == sf::Keyboard::RBracket)
                {
                    if (!allResults.empty())
                    {
                        selectedRun = (selectedRun + 1) % allResults.size();
                    }
                }
                else if (event.key.code == sf::Keyboard::R)
                {
                    rotateMode = !rotateMode;
                }
                else if (event.key.code == sf::Keyboard::A)
                {
                    yaw -= 0.08;
                }
                else if (event.key.code == sf::Keyboard::D)
                {
                    yaw += 0.08;
                }
                else if (event.key.code == sf::Keyboard::W)
                {
                    pitch += 0.08;
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    pitch -= 0.08;
                }
            }
        }

        auto projectPoint = [&](double x, double y, double z)
        {
            if (!rotateMode)
            {
                return toScreen(x, y);
            }

            double cx = x - 0.5;
            double cy = y - 0.5;
            double cz = z - 0.5;

            double cYaw = cos(yaw);
            double sYaw = sin(yaw);
            double cPitch = cos(pitch);
            double sPitch = sin(pitch);

            double x1 = cYaw * cx - sYaw * cy;
            double y1 = sYaw * cx + cYaw * cy;
            double z1 = cz;

            double y2 = cPitch * y1 - sPitch * z1;

            double sx = 0.5 + x1 * 0.95;
            double sy = 0.5 + y2 * 0.95;
            sx = std::max(0.0, std::min(1.0, sx));
            sy = std::max(0.0, std::min(1.0, sy));
            return toScreen(sx, sy);
        };

        ostringstream title;
        title << "Optimizavimas | Zingsnis " << currentStep;
        if (maxPathLen > 0)
        {
            title << "/" << (maxPathLen - 1);
        }
        window.setTitle(title.str());

        window.clear(sf::Color(250, 250, 250));

        sf::RectangleShape sidebarBg(sf::Vector2f(static_cast<float>(SIDEBAR_W), static_cast<float>(WINDOW_HEIGHT - 2 * PADDING)));
        sidebarBg.setPosition(static_cast<float>(SIDEBAR_X), static_cast<float>(PADDING));
        sidebarBg.setFillColor(sf::Color(255, 255, 255, 245));
        sidebarBg.setOutlineColor(sf::Color(180, 180, 180));
        sidebarBg.setOutlineThickness(1.0f);
        window.draw(sidebarBg);

        if (!rotateMode)
        {
            sf::RectangleShape boundary(sf::Vector2f(PLOT_SIZE, 1));
            boundary.setPosition(PADDING, WINDOW_HEIGHT - PADDING);
            boundary.setFillColor(sf::Color::Black);
            window.draw(boundary);

            sf::RectangleShape yAxis(sf::Vector2f(1, PLOT_SIZE));
            yAxis.setPosition(PADDING, PADDING);
            yAxis.setFillColor(sf::Color::Black);
            window.draw(yAxis);

            vector<double> tickValues = {0.0, 0.25, 0.5, 0.75, 1.0};
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

                if (hasOverlayFont)
                {
                    ostringstream val;
                    val << fixed << setprecision(2) << t;

                    sf::Text xVal;
                    xVal.setFont(overlayFont);
                    xVal.setCharacterSize(11);
                    xVal.setFillColor(sf::Color::Black);
                    xVal.setString(val.str());
                    xVal.setPosition(xTickPos.x - 12.0f, xTickPos.y + 8.0f);
                    window.draw(xVal);

                    sf::Text yVal;
                    yVal.setFont(overlayFont);
                    yVal.setCharacterSize(11);
                    yVal.setFillColor(sf::Color::Black);
                    yVal.setString(val.str());
                    yVal.setPosition(yTickPos.x - 34.0f, yTickPos.y - 8.0f);
                    window.draw(yVal);
                }
            }
        }
        else
        {
            auto drawAxis3D = [&](const Point3D &from, const Point3D &to, sf::Color color, float thickness)
            {
                sf::Vector2f p1 = projectPoint(from[0], from[1], from[2]);
                sf::Vector2f p2 = projectPoint(to[0], to[1], to[2]);
                float len = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
                sf::RectangleShape line(sf::Vector2f(len, thickness));
                float angle = atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159f;
                line.setRotation(angle);
                line.setPosition(p1);
                line.setFillColor(color);
                window.draw(line);
            };

            drawAxis3D({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, sf::Color(220, 60, 60), 2.0f);
            drawAxis3D({0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, sf::Color(40, 170, 60), 2.0f);
            drawAxis3D({0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, sf::Color(60, 110, 220), 2.0f);

            if (hasOverlayFont)
            {
                sf::Vector2f ox = projectPoint(0.0, 0.0, 0.0);
                sf::Vector2f px = projectPoint(1.0, 0.0, 0.0);
                sf::Vector2f py = projectPoint(0.0, 1.0, 0.0);
                sf::Vector2f pz = projectPoint(0.0, 0.0, 1.0);

                sf::Text oLbl;
                oLbl.setFont(overlayFont);
                oLbl.setCharacterSize(11);
                oLbl.setFillColor(sf::Color::Black);
                oLbl.setString("0");
                oLbl.setPosition(ox.x + 4.0f, ox.y + 2.0f);
                window.draw(oLbl);

                sf::Text xLbl;
                xLbl.setFont(overlayFont);
                xLbl.setCharacterSize(11);
                xLbl.setFillColor(sf::Color(220, 60, 60));
                xLbl.setString("x=1");
                xLbl.setPosition(px.x + 4.0f, px.y - 6.0f);
                window.draw(xLbl);

                sf::Text yLbl;
                yLbl.setFont(overlayFont);
                yLbl.setCharacterSize(11);
                yLbl.setFillColor(sf::Color(40, 170, 60));
                yLbl.setString("y=1");
                yLbl.setPosition(py.x + 4.0f, py.y - 6.0f);
                window.draw(yLbl);

                sf::Text zLbl;
                zLbl.setFont(overlayFont);
                zLbl.setCharacterSize(11);
                zLbl.setFillColor(sf::Color(60, 110, 220));
                zLbl.setString("z=1");
                zLbl.setPosition(pz.x + 4.0f, pz.y - 6.0f);
                window.draw(zLbl);
            }
        }

        for (size_t i = 0; i < allResults.size(); i++)
        {
            const OptimizationResult &result = allResults[i];
            sf::Color color = runColors[i];
            size_t visibleSteps = min(currentStep + 1, result.path.size());

            for (size_t j = 1; j < visibleSteps; j++)
            {
                double x1 = result.path[j - 1][0];
                double y1 = result.path[j - 1][1];
                double x2 = result.path[j][0];
                double y2 = result.path[j][1];

                double z1 = result.path[j - 1][2];
                double z2 = result.path[j][2];

                sf::Vector2f p1 = projectPoint(x1, y1, z1);
                sf::Vector2f p2 = projectPoint(x2, y2, z2);

                sf::RectangleShape line(sf::Vector2f(
                    sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)), 1));
                float angle = atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159f;
                line.setRotation(angle);
                line.setPosition(p1);
                line.setFillColor(color);
                window.draw(line);
            }

            if (!result.path.empty())
            {
                sf::CircleShape startPoint(5.0f);
                startPoint.setFillColor(sf::Color::Transparent);
                startPoint.setOutlineColor(color);
                startPoint.setOutlineThickness(1.5f);
                sf::Vector2f p = projectPoint(result.path.front()[0], result.path.front()[1], result.path.front()[2]);
                startPoint.setPosition(p.x - 5.0f, p.y - 5.0f);
                window.draw(startPoint);
            }

            if (visibleSteps > 0)
            {
                const vector<double> &current = result.path[visibleSteps - 1];
                sf::CircleShape currentPoint(4.0f);
                currentPoint.setFillColor(color);
                sf::Vector2f p = projectPoint(current[0], current[1], current[2]);
                currentPoint.setPosition(p.x - 4.0f, p.y - 4.0f);
                window.draw(currentPoint);
            }
        }

        sf::CircleShape analyticalMarker(6.0f);
        analyticalMarker.setFillColor(sf::Color::Transparent);
        analyticalMarker.setOutlineColor(sf::Color(0, 0, 0));
        analyticalMarker.setOutlineThickness(2.0f);
        sf::Vector2f analPoint = projectPoint(analyticalSol[0], analyticalSol[1], analyticalSol[2]);
        analyticalMarker.setPosition(analPoint.x - 6.0f, analPoint.y - 6.0f);
        window.draw(analyticalMarker);

        if (hasOverlayFont)
        {
            sf::Text xLabel;
            xLabel.setFont(overlayFont);
            xLabel.setCharacterSize(16);
            xLabel.setFillColor(sf::Color::Black);
            xLabel.setString("x");
            xLabel.setPosition(static_cast<float>(PADDING + PLOT_SIZE + 10), static_cast<float>(WINDOW_HEIGHT - PADDING - 12));
            window.draw(xLabel);

            sf::Text yLabel;
            yLabel.setFont(overlayFont);
            yLabel.setCharacterSize(16);
            yLabel.setFillColor(sf::Color::Black);
            yLabel.setString("y");
            yLabel.setPosition(static_cast<float>(PADDING - 20), static_cast<float>(PADDING - 24));
            window.draw(yLabel);

            sf::RectangleShape hudBg(sf::Vector2f(static_cast<float>(SIDEBAR_W - 20), 152.0f));
            hudBg.setPosition(static_cast<float>(SIDEBAR_X + 10), static_cast<float>(PADDING + 10));
            hudBg.setFillColor(sf::Color(255, 255, 255, 210));
            hudBg.setOutlineColor(sf::Color(120, 120, 120));
            hudBg.setOutlineThickness(1.0f);
            window.draw(hudBg);

            ostringstream hud;
            hud << "Zingsnis: " << currentStep;
            if (maxPathLen > 0)
            {
                hud << " / " << (maxPathLen - 1);
            }
            hud << "\n";
            hud << "Left/Right: +/-1\n";
            hud << "Up/Down: +/-100\n";
            hud << (rotateMode ? "Projekcija: sukinama 3D->2D\n" : "Projekcija: x-y (spausk R)\n");
            hud << "R - sukimas, A/D - yaw\n";
            hud << "W/S - pitch\n";
            hud << "[ / ] - keisti bandyma\n";
            hud << "1-9 - tiesioginis pasirinkimas";

            sf::Text hudText;
            hudText.setFont(overlayFont);
            hudText.setCharacterSize(13);
            hudText.setFillColor(sf::Color::Black);
            hudText.setPosition(static_cast<float>(SIDEBAR_X + 18), static_cast<float>(PADDING + 16));
            hudText.setString(hud.str());
            window.draw(hudText);

            // Legend panel is in sidebar to avoid overlapping plot elements.
            const float legendW = static_cast<float>(SIDEBAR_W - 20);
            const float legendH = 210.0f;
            const float legendX = static_cast<float>(SIDEBAR_X + 10);
            const float legendY = static_cast<float>(PADDING + 174);
            sf::RectangleShape legendBg(sf::Vector2f(legendW, legendH));
            legendBg.setPosition(legendX, legendY);
            legendBg.setFillColor(sf::Color(255, 255, 255, 230));
            legendBg.setOutlineColor(sf::Color(120, 120, 120));
            legendBg.setOutlineThickness(1.0f);
            window.draw(legendBg);

            sf::Text legendTitle;
            legendTitle.setFont(overlayFont);
            legendTitle.setCharacterSize(13);
            legendTitle.setFillColor(sf::Color::Black);
            legendTitle.setString("Legenda");
            legendTitle.setPosition(legendX + 8.0f, legendY + 6.0f);
            window.draw(legendTitle);

            // Draw swatches for runs (colors vector defined earlier)
            const float runBaseY = legendY + 28.0f;
            const float runStep = 19.0f;
            for (size_t i = 0; i < allResults.size() && i < 5; ++i)
            {
                sf::RectangleShape sw(sf::Vector2f(18.0f, 12.0f));
                sf::Color col = runColors[i];
                sw.setFillColor(col);
                sw.setPosition(legendX + 10.0f, runBaseY + static_cast<float>(i) * runStep);
                window.draw(sw);

                sf::Text lbl;
                lbl.setFont(overlayFont);
                lbl.setCharacterSize(12);
                lbl.setFillColor(sf::Color::Black);
                ostringstream lstr;
                lstr << "Bandymas " << (i + 1) << "  r=" << scientific << setprecision(1) << allResults[i].penaltyParameter;
                lbl.setString(lstr.str());
                lbl.setPosition(legendX + 34.0f, runBaseY - 2.0f + static_cast<float>(i) * runStep);
                window.draw(lbl);
            }

            if (allResults.size() > 5)
            {
                sf::Text moreLbl;
                moreLbl.setFont(overlayFont);
                moreLbl.setCharacterSize(11);
                moreLbl.setFillColor(sf::Color::Black);
                moreLbl.setString("Bandymai nuo 6 rodomi tik grafike");
                moreLbl.setPosition(legendX + 10.0f, runBaseY + 5.0f * runStep + 2.0f);
                window.draw(moreLbl);
            }

            // Marker legend
            sf::CircleShape filled(6.0f);
            filled.setFillColor(sf::Color::Black);
            filled.setPosition(legendX + 10.0f, legendY + legendH - 56.0f);
            window.draw(filled);
            sf::Text filledLbl;
            filledLbl.setFont(overlayFont);
            filledLbl.setCharacterSize(12);
            filledLbl.setFillColor(sf::Color::Black);
            filledLbl.setString("Dabartinis taskas (uzpildytas)");
            filledLbl.setPosition(legendX + 28.0f, legendY + legendH - 58.0f);
            window.draw(filledLbl);

            sf::CircleShape ring(6.0f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color::Black);
            ring.setOutlineThickness(1.4f);
            ring.setPosition(legendX + 10.0f, legendY + legendH - 34.0f);
            window.draw(ring);
            sf::Text ringLbl;
            ringLbl.setFont(overlayFont);
            ringLbl.setCharacterSize(12);
            ringLbl.setFillColor(sf::Color::Black);
            ringLbl.setString("Analitinis optimumas (ziedas)");
            ringLbl.setPosition(legendX + 28.0f, legendY + legendH - 36.0f);
            window.draw(ringLbl);

            // Draw projected gradient arrow at tracked point
            selectedRun = min(selectedRun, allResults.size() > 0 ? allResults.size() - 1 : 0);
            const OptimizationResult &tracked = allResults[selectedRun];
            size_t visibleTracked = min(currentStep + 1, tracked.path.size());
            if (visibleTracked > 0)
            {
                double gx = 0.0, gy = 0.0;
                Point3D curPoint = {tracked.path[visibleTracked - 1][0], tracked.path[visibleTracked - 1][1], tracked.path[visibleTracked - 1][2]};
                Point3D g = penaltyGradient(curPoint, tracked.penaltyParameter);
                // project gradient into xy plane and scale for visibility
                gx = static_cast<double>(g[0]);
                gy = static_cast<double>(g[1]);
                double gnorm = sqrt(gx * gx + gy * gy);
                if (gnorm > 1e-12)
                {
                    double scaleVis = 0.06 * PLOT_SIZE / gnorm; // visual scaling
                    sf::Vector2f base = projectPoint(curPoint[0], curPoint[1], curPoint[2]);
                    sf::Vector2f tip = projectPoint(curPoint[0] + gx * scaleVis, curPoint[1] + gy * scaleVis, curPoint[2] + g[2] * scaleVis);
                    sf::RectangleShape arrow(sf::Vector2f(sqrt(pow(tip.x - base.x, 2) + pow(tip.y - base.y, 2)), 2.0f));
                    float angle = atan2(tip.y - base.y, tip.x - base.x) * 180.0f / 3.14159f;
                    arrow.setRotation(angle);
                    arrow.setPosition(base);
                    arrow.setFillColor(sf::Color(40, 40, 40));
                    window.draw(arrow);

                    // arrow head
                    sf::ConvexShape ah;
                    ah.setPointCount(3);
                    ah.setPoint(0, sf::Vector2f(0.0f, 0.0f));
                    ah.setPoint(1, sf::Vector2f(-8.0f, -4.0f));
                    ah.setPoint(2, sf::Vector2f(-8.0f, 4.0f));
                    ah.setFillColor(sf::Color(40, 40, 40));
                    ah.setPosition(tip);
                    ah.setRotation(angle);
                    window.draw(ah);
                }
            }

            sf::RectangleShape infoPanelBg(sf::Vector2f(static_cast<float>(SIDEBAR_W - 20), 110.0f));
            infoPanelBg.setPosition(static_cast<float>(SIDEBAR_X + 10), legendY + legendH + 10.0f);
            infoPanelBg.setFillColor(sf::Color(255, 255, 255, 210));
            infoPanelBg.setOutlineColor(sf::Color(120, 120, 120));
            infoPanelBg.setOutlineThickness(1.0f);
            window.draw(infoPanelBg);

            if (visibleTracked > 0)
            {
                double xCur = tracked.path[visibleTracked - 1][0];
                double yCur = tracked.path[visibleTracked - 1][1];
                double zCur = tracked.path[visibleTracked - 1][2];

                ostringstream panel;
                panel << "Bandymas " << (selectedRun + 1) << ": r=" << scientific << setprecision(2) << tracked.penaltyParameter << "\n";
                panel << "x=" << fixed << setprecision(5) << xCur << "  y=" << yCur << "  z=" << zCur;

                sf::Text panelText;
                panelText.setFont(overlayFont);
                panelText.setCharacterSize(13);
                panelText.setFillColor(sf::Color::Black);
                panelText.setPosition(static_cast<float>(SIDEBAR_X + 18), legendY + legendH + 18.0f);
                panelText.setString(panel.str());
                window.draw(panelText);
            }
        }

        window.display();
    }
}

int main()
{
    cout << fixed << setprecision(8);
    cout << "=== Dezes turio optimizavimas kvadratinio baudos metodo budu ===" << endl;
    cout << "Studento skaiciai: a = " << a << ", b = " << b << ", c = " << c << endl
         << endl;

    const Point3D X0 = {0.0, 0.0, 0.0};
    const Point3D X1 = {1.0, 1.0, 1.0};
    const Point3D Xm = {a / 10.0, b / 10.0, c / 10.0};

    cout << "=== Pradiniai taskai ===" << endl;
    cout << "  X0 = " << formatPoint(X0) << endl;
    cout << "  X1 = " << formatPoint(X1) << endl;
    cout << "  Xm = " << formatPoint(Xm, 8) << endl
         << endl;

    cout << "=== Apribojimu reiksmes pradiniuose taskuose ===" << endl;
    const Point3D startPoints[] = {X0, X1, Xm};
    const string startNames[] = {"X0", "X1", "Xm"};

    for (int i = 0; i < 3; i++)
    {
        cout << startNames[i] << " = " << formatPoint(startPoints[i]) << endl;
        cout << "  f(X) = " << objectiveFunction(startPoints[i]) << endl;
        cout << "  g(X) = " << equalityConstraint(startPoints[i]) << endl;
        const auto ineq = inequalityConstraints(startPoints[i]);
        cout << "  h_1(X) = " << ineq[0] << ", h_2(X) = " << ineq[1] << ", h_3(X) = " << ineq[2] << endl;
        cout << "  Baudos pazeidimas = " << penaltyViolation(startPoints[i]) << endl
             << endl;
    }

    cout << "=== Analitinis sprendinys ===" << endl;
    const Point3D analytical = analyticalSolution();
    cout << "  X* = " << formatPoint(analytical) << endl;
    cout << "  f(X*) = " << analyticalMinimumValue() << endl;
    cout << "  Tenkina apribojimus: " << (isFeasiblePoint(analytical) ? "taip" : "ne") << endl
         << endl;

    vector<OptimizationResult> allResults;

    const vector<double> penaltySequence = {1.0, 0.1, 0.01, 0.001, 0.0001};

    cout << "=== Vykdoma baudos metodo seka ===" << endl;

    for (const Point3D &startPoint : startPoints)
    {
        Point3D currentStart = startPoint;
        cout << "\nOptimizavimo seka nuo tasko " << formatPoint(startPoint) << ":" << endl;

        for (double r : penaltySequence)
        {
            const auto result = minimizePenaltyDirectSearch(currentStart, r, 1e-8, 20000, 0.25);
            cout << "  r = " << scientific << setprecision(4) << r << ": "
                 << "f(X) = " << fixed << setprecision(8) << result.objectiveValue
                 << ", zingsniai = " << result.steps
                 << ", evals = " << result.functionEvaluations
                 << ", tenkina = " << (isFeasiblePoint(result.solution) ? "taip" : "ne") << endl;

            allResults.push_back(result);
            currentStart = result.solution;
        }
    }

    cout << "\n=== Santrauka ===" << endl;
    cout << "Bandymu skaicius: " << allResults.size() << endl;
    cout << "Rezultatai issaugoti CSV failuose." << endl
         << endl;

    savePathsToCsv(allResults);

    cout << "=== Vizualizacija ===" << endl;
    cout << "Ar norite parodyti optimizavimo kelius? (y/n): ";
    char response;
    cin >> response;

    if (response == 'y' || response == 'Y')
    {
        visualizeResults(allResults);
    }
    else
    {
        cout << "Vizualizacija praleista." << endl;
    }

    return 0;
}
