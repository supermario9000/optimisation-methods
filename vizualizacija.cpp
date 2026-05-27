#include "lib.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sstream>

namespace {

constexpr int WINDOW_W = 1340;
constexpr int WINDOW_H = 800;
constexpr int OUTER_PAD = 30;
constexpr int PANEL_GAP = 40;
constexpr int PANEL_W = (WINDOW_W - 2 * OUTER_PAD - PANEL_GAP) / 2;
constexpr int TITLE_TOP = 15;
constexpr int PLOT_TOP = 66;
constexpr int LEFT_PAD = 70; // within a panel
constexpr int RIGHT_PAD = 30;
constexpr int BOTTOM_HUD = 190;

struct PanelLayout
{
    int plotX, plotY, plotW, plotH;
    double x2Max, x4Max;
};

PanelLayout makePanel(int panelX, double x2Max, double x4Max)
{
    PanelLayout p;
    p.plotX = panelX + LEFT_PAD;
    p.plotY = PLOT_TOP + 28;
    p.plotW = PANEL_W - LEFT_PAD - RIGHT_PAD;
    p.plotH = WINDOW_H - p.plotY - BOTTOM_HUD;
    p.x2Max = x2Max;
    p.x4Max = x4Max;
    return p;
}

sf::Vector2f toScreen(const PanelLayout &p, double x2, double x4)
{
    return sf::Vector2f(
        static_cast<float>(p.plotX + (x2 / p.x2Max) * p.plotW),
        static_cast<float>(p.plotY + p.plotH - (x4 / p.x4Max) * p.plotH));
}

sf::Color heatColor(double t)
{
    t = max(0.0, min(1.0, t));
    sf::Uint8 r = static_cast<sf::Uint8>(245 - 130 * t);
    sf::Uint8 g = static_cast<sf::Uint8>(245 - 150 * t);
    sf::Uint8 bl = static_cast<sf::Uint8>(255 - 80 * t);
    return sf::Color(r, g, bl);
}

void drawText(sf::RenderWindow &window, sf::Font &font, const string &s,
              float x, float y, unsigned size, sf::Color color, bool bold = false)
{
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setFillColor(color);
    if (bold)
        t.setStyle(sf::Text::Bold);
    t.setString(s);
    t.setPosition(x, y);
    window.draw(t);
}

string fmt(double v, int prec = 4)
{
    ostringstream os;
    os << fixed << setprecision(prec) << v;
    return os.str();
}

void drawPanel(sf::RenderWindow &window, sf::Font &font, bool hasFont,
               const LPProblem &lp, const SimplexResult &res,
               int currentStep, int panelX)
{
    double feasX2 = lp.b[1] / 4.0;
    double feasX4 = lp.b[2];
    double x2Max = feasX2 * 1.25 + 0.2;
    double x4Max = feasX4 * 1.15 + 0.5;

    PanelLayout p = makePanel(panelX, x2Max, x4Max);

    if (hasFont)
        drawText(window, font, lp.name,
                 static_cast<float>(panelX + 20), static_cast<float>(PLOT_TOP),
                 17, sf::Color::Black, true);

    double fMin = -3.0 * feasX2 - 5.0 * feasX4;
    double fMax = 0.0;
    double fRange = fMax - fMin;
    if (fRange < 1e-12)
        fRange = 1.0;

    // Heatmap.
    for (int sx = p.plotX; sx <= p.plotX + p.plotW; sx += 3)
    {
        for (int sy = p.plotY; sy <= p.plotY + p.plotH; sy += 3)
        {
            double x2 = (sx - p.plotX) / static_cast<double>(p.plotW) * p.x2Max;
            double x4 = (p.plotY + p.plotH - sy) / static_cast<double>(p.plotH) * p.x4Max;
            if (x2 >= 0 && x2 <= feasX2 && x4 >= 0 && x4 <= feasX4 &&
                (x2 - x4) <= lp.b[0])
            {
                double f = -3.0 * x2 - 5.0 * x4;
                double t = (fMax - f) / fRange;
                sf::RectangleShape cell(sf::Vector2f(3.0f, 3.0f));
                cell.setFillColor(heatColor(t));
                cell.setPosition(static_cast<float>(sx), static_cast<float>(sy));
                window.draw(cell);
            }
        }
    }

    // Feasible region outline.
    sf::Vector2f corners[4] = {
        toScreen(p, 0, 0),
        toScreen(p, feasX2, 0),
        toScreen(p, feasX2, feasX4),
        toScreen(p, 0, feasX4)};
    for (int i = 0; i < 4; i++)
    {
        sf::Vertex line[] = {
            sf::Vertex(corners[i], sf::Color(90, 90, 90)),
            sf::Vertex(corners[(i + 1) % 4], sf::Color(90, 90, 90))};
        window.draw(line, 2, sf::Lines);
    }

    // Axes.
    sf::RectangleShape xAxis(sf::Vector2f(static_cast<float>(p.plotW), 1.5f));
    xAxis.setPosition(static_cast<float>(p.plotX), static_cast<float>(p.plotY + p.plotH));
    xAxis.setFillColor(sf::Color::Black);
    window.draw(xAxis);

    sf::RectangleShape yAxis(sf::Vector2f(1.5f, static_cast<float>(p.plotH)));
    yAxis.setPosition(static_cast<float>(p.plotX), static_cast<float>(p.plotY));
    yAxis.setFillColor(sf::Color::Black);
    window.draw(yAxis);

    // Ticks + labels.
    vector<double> xTicks = {0.0, feasX2 / 2.0, feasX2};
    vector<double> yTicks = {0.0, feasX4 / 2.0, feasX4};
    for (double t : xTicks)
    {
        sf::Vector2f pos = toScreen(p, t, 0);
        sf::RectangleShape tick(sf::Vector2f(1.0f, 6.0f));
        tick.setPosition(pos.x, pos.y - 3.0f);
        tick.setFillColor(sf::Color::Black);
        window.draw(tick);
        if (hasFont)
            drawText(window, font, fmt(t, 2), pos.x - 12.0f, pos.y + 8.0f, 11, sf::Color::Black);
    }
    for (double t : yTicks)
    {
        sf::Vector2f pos = toScreen(p, 0, t);
        sf::RectangleShape tick(sf::Vector2f(6.0f, 1.0f));
        tick.setPosition(pos.x - 3.0f, pos.y);
        tick.setFillColor(sf::Color::Black);
        window.draw(tick);
        if (hasFont)
            drawText(window, font, fmt(t, 2), pos.x - 50.0f, pos.y - 8.0f, 11, sf::Color::Black);
    }
    if (hasFont)
    {
        drawText(window, font, "x2",
                 static_cast<float>(p.plotX + p.plotW / 2 - 8),
                 static_cast<float>(p.plotY + p.plotH + 28),
                 14, sf::Color::Black);
        drawText(window, font, "x4",
                 static_cast<float>(p.plotX - 60),
                 static_cast<float>(p.plotY + p.plotH / 2 - 8),
                 14, sf::Color::Black);
    }

    // Pivot path up to currentStep.
    if (!res.steps.empty())
    {
        sf::Color pathColor(200, 0, 0);
        int lastIdx = min(currentStep, static_cast<int>(res.steps.size()) - 1);

        for (int i = 1; i <= lastIdx; i++)
        {
            const Vector &prev = res.steps[i - 1].point;
            const Vector &curr = res.steps[i].point;
            sf::Vector2f a = toScreen(p, prev[1], prev[3]);
            sf::Vector2f cc = toScreen(p, curr[1], curr[3]);
            float dx = cc.x - a.x;
            float dy = cc.y - a.y;
            float len = sqrt(dx * dx + dy * dy);
            if (len < 0.5f)
                continue;
            sf::RectangleShape line(sf::Vector2f(len, 2.5f));
            float angle = atan2(dy, dx) * 180.0f / 3.14159f;
            line.setRotation(angle);
            line.setPosition(a);
            line.setFillColor(pathColor);
            window.draw(line);

            if (len > 14.0f)
            {
                sf::ConvexShape arrow;
                arrow.setPointCount(3);
                arrow.setPoint(0, sf::Vector2f(0, 0));
                arrow.setPoint(1, sf::Vector2f(-9.0f, -4.5f));
                arrow.setPoint(2, sf::Vector2f(-9.0f, 4.5f));
                arrow.setFillColor(pathColor);
                arrow.setPosition(cc);
                arrow.setRotation(angle);
                window.draw(arrow);
            }
        }

        int totalSteps = static_cast<int>(res.steps.size());
        for (int i = 0; i <= lastIdx; i++)
        {
            sf::Vector2f pos = toScreen(p, res.steps[i].point[1], res.steps[i].point[3]);
            if (i == 0)
            {
                sf::RectangleShape sq(sf::Vector2f(12.0f, 12.0f));
                sq.setFillColor(sf::Color::Transparent);
                sq.setOutlineColor(sf::Color(0, 130, 0));
                sq.setOutlineThickness(2.0f);
                sq.setPosition(pos.x - 6.0f, pos.y - 6.0f);
                window.draw(sq);
            }
            else if (i == totalSteps - 1)
            {
                sf::CircleShape c(8.5f);
                c.setFillColor(sf::Color(200, 0, 0));
                c.setOutlineColor(sf::Color::Black);
                c.setOutlineThickness(1.0f);
                c.setPosition(pos.x - 8.5f, pos.y - 8.5f);
                window.draw(c);
            }
            else
            {
                sf::CircleShape d(5.0f);
                d.setFillColor(sf::Color(200, 0, 0));
                d.setPosition(pos.x - 5.0f, pos.y - 5.0f);
                window.draw(d);
            }
        }

        // Highlight the current step with a yellow ring.
        sf::Vector2f curPos = toScreen(p,
                                       res.steps[lastIdx].point[1],
                                       res.steps[lastIdx].point[3]);
        sf::CircleShape ring(14.0f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(255, 180, 0));
        ring.setOutlineThickness(2.5f);
        ring.setPosition(curPos.x - 14.0f, curPos.y - 14.0f);
        window.draw(ring);
    }

    // HUD beneath the plot.
    if (hasFont && !res.steps.empty())
    {
        int lastIdx = min(currentStep, static_cast<int>(res.steps.size()) - 1);
        const SimplexStep &cur = res.steps[lastIdx];
        int totalSteps = static_cast<int>(res.steps.size());

        float hudX = static_cast<float>(panelX + 20);
        float hudY = static_cast<float>(p.plotY + p.plotH + 46);

        ostringstream stepLine;
        stepLine << "Step " << lastIdx << " / " << (totalSteps - 1) << "  :  ";
        if (cur.enteringVar < 0)
            stepLine << "initial vertex";
        else
            stepLine << "x" << (cur.enteringVar + 1) << " enters, x"
                     << (cur.leavingVar + 1) << " leaves";
        if (lastIdx == totalSteps - 1 && res.optimal)
            stepLine << "  [OPTIMAL]";
        drawText(window, font, stepLine.str(), hudX, hudY, 15, sf::Color::Black, true);

        drawText(window, font, "z = " + fmt(cur.z),
                 hudX, hudY + 23, 13, sf::Color::Black);

        ostringstream xs;
        xs << "x1=" << fmt(cur.point[0]) << "  x2=" << fmt(cur.point[1])
           << "  x3=" << fmt(cur.point[2]) << "  x4=" << fmt(cur.point[3]);
        drawText(window, font, xs.str(), hudX, hudY + 44, 13, sf::Color::Black);

        ostringstream ss;
        ss << "x5=" << fmt(cur.point[4]) << "  x6=" << fmt(cur.point[5])
           << "  x7=" << fmt(cur.point[6]);
        drawText(window, font, ss.str(), hudX, hudY + 64, 13, sf::Color::Black);

        string basis = "Basis: {";
        for (size_t i = 0; i < cur.basis.size(); i++)
        {
            basis += "x" + to_string(cur.basis[i] + 1);
            if (i + 1 < cur.basis.size())
                basis += ", ";
        }
        basis += "}";
        drawText(window, font, basis, hudX, hudY + 84, 13, sf::Color::Black);

        drawText(window, font,
                 "b = (" + fmt(lp.b[0], 1) + ", " + fmt(lp.b[1], 1) + ", " + fmt(lp.b[2], 1) + ")",
                 hudX, hudY + 104, 12, sf::Color(80, 80, 80));
    }
}

} // namespace

void visualizeLP(const LPProblem &lp1, const SimplexResult &r1,
                 const LPProblem &lp2, const SimplexResult &r2)
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H),
                            "Simplex visualization");
    window.setFramerateLimit(60);

    sf::Font font;
    bool hasFont = false;
    vector<string> fontCandidates = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf"};
    for (const auto &fp : fontCandidates)
    {
        if (font.loadFromFile(fp))
        {
            hasFont = true;
            break;
        }
    }

    cout << "\nOpening visualization window (official vs individual, side by side)...\n";
    cout << "Controls:\n";
    cout << "  Right / Left arrows : advance / rewind one simplex step (both panels)\n";
    cout << "  Home / End          : jump to first / last step\n";
    cout << "  Esc or close button : exit\n";
    cout << "Legend:\n";
    cout << "  Hollow green square = start vertex (origin)\n";
    cout << "  Red line + arrow    = simplex pivot edge\n";
    cout << "  Filled red circle   = optimal vertex\n";
    cout << "  Yellow ring         = currently selected step\n";
    cout << "  Heatmap             = objective z = -3 x2 - 5 x4 (darker = lower = better)\n";

    int maxSteps = max(static_cast<int>(r1.steps.size()),
                       static_cast<int>(r2.steps.size()));
    int currentStep = 0;

    int panel1X = OUTER_PAD;
    int panel2X = OUTER_PAD + PANEL_W + PANEL_GAP;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (event.key.code == sf::Keyboard::Right)
                    currentStep = min(currentStep + 1, maxSteps - 1);
                else if (event.key.code == sf::Keyboard::Left)
                    currentStep = max(currentStep - 1, 0);
                else if (event.key.code == sf::Keyboard::Home)
                    currentStep = 0;
                else if (event.key.code == sf::Keyboard::End)
                    currentStep = maxSteps - 1;
            }
        }

        ostringstream title;
        title << "Simplex visualization  |  Step " << currentStep
              << " / " << (maxSteps - 1);
        window.setTitle(title.str());

        window.clear(sf::Color::White);

        if (hasFont)
            drawText(window, font,
                     "Simplex visualization: feasible region and pivot path in (x2, x4)  --  use Left/Right to step, Home/End to jump, Esc to close",
                     static_cast<float>(OUTER_PAD), static_cast<float>(TITLE_TOP),
                     17, sf::Color::Black, true);

        drawPanel(window, font, hasFont, lp1, r1, currentStep, panel1X);
        drawPanel(window, font, hasFont, lp2, r2, currentStep, panel2X);

        // Separator between panels.
        sf::RectangleShape sep(sf::Vector2f(1.0f, static_cast<float>(WINDOW_H - PLOT_TOP - 10)));
        sep.setPosition(static_cast<float>(OUTER_PAD + PANEL_W + PANEL_GAP / 2),
                        static_cast<float>(PLOT_TOP - 4));
        sep.setFillColor(sf::Color(210, 210, 210));
        window.draw(sep);

        window.display();
    }
}
