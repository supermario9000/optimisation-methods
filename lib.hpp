#ifndef LIB_HPP
#define LIB_HPP

#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace std;

// Configuration variables - set these with your student ID values
extern int a;
extern int b;

// Result structure for storing algorithm outcomes
struct OptimizationResult {
    vector<double> solution;           // Final point (x, y)
    double minValue;                   // Minimum function value found
    int steps;                         // Number of iterations
    int functionEvaluations;           // Number of function evaluations
    string algorithmName;              // Name of algorithm used
    double startingX, startingY;       // Starting point
    vector<vector<double>> path;       // Path of points visited
};

// Objective function: f(x, y) = -x*y*(1-x-y)/8
inline double objectiveFunction(double x, double y) {
    return -x * y * (1.0 - x - y) / 8.0;
}

// Gradient of objective function
// df/dx = -y*(1-2x-y)/8
// df/dy = -x*(1-x-2y)/8
inline void computeGradient(double x, double y, double& gradX, double& gradY) {
    gradX = -y * (1.0 - 2.0 * x - y) / 8.0;
    gradY = -x * (1.0 - x - 2.0 * y) / 8.0;
}

// Compute gradient magnitude (norm)
inline double gradientMagnitude(double x, double y) {
    double gradX, gradY;
    computeGradient(x, y, gradX, gradY);
    return sqrt(gradX * gradX + gradY * gradY);
}

// Clamp values to valid range [0, 1]
inline double clamp(double value) {
    return max(0.0, min(1.0, value));
}

// Check if point is in valid region (x + y <= 1)
inline bool isValidPoint(double x, double y) {
    return x >= 0.0 && y >= 0.0 && (x + y) <= 1.0;
}

// Distance between two points
inline double distance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

#endif

