#ifndef LIB_HPP
#define LIB_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

using Point3D = array<double, 3>;

extern int a;
extern int b;
extern int c;

struct OptimizationResult
{
    string algorithmName;
    double penaltyParameter = 0.0;
    Point3D startingPoint{};
    Point3D solution{};
    double objectiveValue = 0.0;
    double penaltyValue = 0.0;
    double equalityValue = 0.0;
    vector<double> inequalityValues;
    int steps = 0;
    int functionEvaluations = 0;
    vector<vector<double>> path;
};

inline Point3D makePoint(double x, double y, double z)
{
    return {x, y, z};
}

inline vector<double> toVector(const Point3D &point)
{
    return {point[0], point[1], point[2]};
}

inline double dotProduct(const Point3D &lhs, const Point3D &rhs)
{
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

inline double norm(const Point3D &point)
{
    return sqrt(dotProduct(point, point));
}

inline Point3D add(const Point3D &lhs, const Point3D &rhs)
{
    return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

inline Point3D subtract(const Point3D &lhs, const Point3D &rhs)
{
    return {lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]};
}

inline Point3D scale(const Point3D &point, double factor)
{
    return {point[0] * factor, point[1] * factor, point[2] * factor};
}

inline string formatPoint(const Point3D &point, int precision = 6)
{
    ostringstream out;
    out << fixed << setprecision(precision)
        << '(' << point[0] << ", " << point[1] << ", " << point[2] << ')';
    return out.str();
}

inline double objectiveFunction(const Point3D &point)
{
    return -point[0] * point[1] * point[2];
}

inline double equalityConstraint(const Point3D &point)
{
    return 2.0 * (point[0] * point[1] + point[1] * point[2] + point[0] * point[2]) - 1.0;
}

inline vector<double> inequalityConstraints(const Point3D &point)
{
    return {-point[0], -point[1], -point[2]};
}

inline double positivePart(double value)
{
    return max(0.0, value);
}

inline double penaltyViolation(const Point3D &point)
{
    double violation = equalityConstraint(point) * equalityConstraint(point);
    for (double value : inequalityConstraints(point))
    {
        double positive = positivePart(value);
        violation += positive * positive;
    }
    return violation;
}

inline double penaltyFunction(const Point3D &point, double r)
{
    return objectiveFunction(point) + penaltyViolation(point) / r;
}

inline Point3D objectiveGradient(const Point3D &point)
{
    return {-point[1] * point[2], -point[0] * point[2], -point[0] * point[1]};
}

inline Point3D penaltyGradient(const Point3D &point, double r)
{
    Point3D grad = objectiveGradient(point);

    const double equality = equalityConstraint(point);
    const Point3D gradEquality = {
        2.0 * (point[1] + point[2]),
        2.0 * (point[0] + point[2]),
        2.0 * (point[0] + point[1])};
    grad = add(grad, scale(gradEquality, (2.0 * equality) / r));

    const vector<double> inequalities = inequalityConstraints(point);
    for (size_t i = 0; i < inequalities.size(); ++i)
    {
        if (inequalities[i] > 0.0)
        {
            Point3D contribution{};
            contribution[i] = 1.0;
            grad = add(grad, scale(contribution, (-2.0 * inequalities[i]) / r));
        }
    }

    return grad;
}

inline bool isFeasiblePoint(const Point3D &point, double tolerance = 1e-12)
{
    const vector<double> inequalities = inequalityConstraints(point);
    return fabs(equalityConstraint(point)) <= tolerance &&
           all_of(inequalities.begin(), inequalities.end(), [&](double value)
                  { return value <= tolerance; });
}

inline Point3D analyticalSolution()
{
    const double edge = 1.0 / sqrt(6.0);
    return {edge, edge, edge};
}

inline double analyticalMinimumValue()
{
    const double edge = 1.0 / sqrt(6.0);
    return -edge * edge * edge;
}

inline double euclideanDistance(const Point3D &lhs, const Point3D &rhs)
{
    return norm(subtract(lhs, rhs));
}

inline vector<Point3D> directSearchDirections()
{
    const double invSqrt3 = 1.0 / sqrt(3.0);
    return {
        {1.0, 0.0, 0.0},
        {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 0.0, -1.0},
        {invSqrt3, invSqrt3, invSqrt3},
        {invSqrt3, invSqrt3, -invSqrt3},
        {invSqrt3, -invSqrt3, invSqrt3},
        {-invSqrt3, invSqrt3, invSqrt3},
        {invSqrt3, -invSqrt3, -invSqrt3},
        {-invSqrt3, invSqrt3, -invSqrt3},
        {-invSqrt3, -invSqrt3, invSqrt3},
        {-invSqrt3, -invSqrt3, -invSqrt3}};
}

OptimizationResult minimizePenaltyDirectSearch(const Point3D &startPoint,
                                               double penaltyParameter,
                                               double tolerance = 1e-8,
                                               int maxIterations = 20000,
                                               double initialStep = 0.25);

OptimizationResult minimizePenaltySteepestDescent(const Point3D &startPoint,
                                                  double penaltyParameter,
                                                  double tolerance = 1e-8,
                                                  int maxIterations = 20000,
                                                  double initialStep = 0.25);

OptimizationResult minimizePenaltyNelderMead(const Point3D &startPoint,
                                             double penaltyParameter,
                                             double tolerance = 1e-8,
                                             int maxIterations = 20000,
                                             double initialStep = 0.05);

#endif
