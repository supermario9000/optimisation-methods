#include "lib.hpp"

OptimizationResult minimizePenaltyDirectSearch(const Point3D &startPoint,
                                               double penaltyParameter,
                                               double tolerance,
                                               int maxIterations,
                                               double initialStep)
{
    OptimizationResult result;
    result.algorithmName = "Penalty direct search";
    result.penaltyParameter = penaltyParameter;
    result.startingPoint = startPoint;
    result.solution = startPoint;

    Point3D current = startPoint;
    double currentValue = penaltyFunction(current, penaltyParameter);
    result.functionEvaluations++;
    result.path.push_back(toVector(current));

    const vector<Point3D> directions = directSearchDirections();
    double step = initialStep;

    for (int iteration = 0; iteration < maxIterations && step >= tolerance; ++iteration)
    {
        Point3D bestPoint = current;
        double bestValue = currentValue;
        bool improved = false;

        for (const Point3D &direction : directions)
        {
            const Point3D candidate = add(current, scale(direction, step));
            const double candidateValue = penaltyFunction(candidate, penaltyParameter);
            result.functionEvaluations++;

            if (candidateValue < bestValue - 1e-14)
            {
                bestValue = candidateValue;
                bestPoint = candidate;
                improved = true;
            }
        }

        if (improved)
        {
            const Point3D previous = current;
            current = bestPoint;
            currentValue = bestValue;
            result.path.push_back(toVector(current));
            result.steps++;

            const Point3D patternPoint = add(current, subtract(current, previous));
            const double patternValue = penaltyFunction(patternPoint, penaltyParameter);
            result.functionEvaluations++;

            if (patternValue < currentValue - 1e-14)
            {
                current = patternPoint;
                currentValue = patternValue;
                result.path.push_back(toVector(current));
                result.steps++;
                step = min(step * 1.10, initialStep * 4.0);
            }
        }
        else
        {
            step *= 0.5;
        }
    }

    result.solution = current;
    result.objectiveValue = objectiveFunction(current);
    result.penaltyValue = currentValue;
    result.equalityValue = equalityConstraint(current);
    result.inequalityValues = inequalityConstraints(current);

    return result;
}
