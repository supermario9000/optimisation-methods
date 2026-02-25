#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace std;

inline constexpr double a = 3; //is mano stud knygeles
inline constexpr double b = 7;
inline constexpr double eps = 0.0001; //paklaida X asies

inline double f(double x) //pagrindine funkcija
{
    return (pow((pow(x,2)-a),2) / b - 1 );//f(x)=(x^2-a)^2/b-1
}

inline double df(double x) //pirmas funkcijos darinys
{
    return (4*x*pow((pow(x,2)-a),1) / b);//f'(x)=4x(x^2-a)/b
}

inline double ddf(double x) //antras funkcijos darinys
{
    return (4*(3*pow(x,2)-a) / b);//f''(x)=4(3x^2-a)/b
}

void IDPmetodas(double r, double l);
void AuksoPjuvioAlgoritmas(double r, double l);
void NiutonoMetodas();

//graphic functions
void openWindow();
void drawAxes(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale, const sf::Font& font);
void drawFunction(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale);
void drawPoints(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, double yScale, const std::vector<double>& xs, const sf::Color& color);
void drawGraph(sf::RenderWindow& window, float offsetX, float offsetY, float width, float height, double minX, double maxX, const sf::Font& font, const std::vector<double>& points, const sf::Color& pointColor, double yScale);