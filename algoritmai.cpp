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

OptimizationResult minimizePenaltySteepestDescent(const Point3D &startPoint,
                                                  double penaltyParameter,
                                                  double tolerance,
                                                  int maxIterations,
                                                  double initialStep)
{
    OptimizationResult result;
    result.algorithmName = "Penalty steepest descent";
    result.penaltyParameter = penaltyParameter;
    result.startingPoint = startPoint;
    result.solution = startPoint;

    Point3D current = startPoint;
    double currentValue = penaltyFunction(current, penaltyParameter);
    result.functionEvaluations++;
    result.path.push_back(toVector(current));

    double alpha0 = initialStep;

    for (int iteration = 0; iteration < maxIterations; ++iteration)
    {
        Point3D g = penaltyGradient(current, penaltyParameter);
        double gnorm = norm(g);
        if (gnorm <= tolerance)
            break;

        // Steepest descent direction
        Point3D dir = scale(g, -1.0);

        // Backtracking Armijo line search
        double alpha = alpha0;
        const double c = 1e-4;
        const double rho = 0.5;
        double dg = dotProduct(g, dir);

        bool found = false;
        for (int ls = 0; ls < 60; ++ls)
        {
            Point3D candidate = add(current, scale(dir, alpha));
            double candVal = penaltyFunction(candidate, penaltyParameter);
            result.functionEvaluations++;
            if (candVal <= currentValue + c * alpha * dg)
            {
                current = candidate;
                currentValue = candVal;
                result.path.push_back(toVector(current));
                result.steps++;
                found = true;
                break;
            }
            alpha *= rho;
            if (alpha < 1e-16)
                break;
        }

        if (!found)
        {
            // Cannot find a satisfactory step -> reduce initial step and try again
            alpha0 *= 0.5;
            if (alpha0 < 1e-16)
                break;
        }
    }

    result.solution = current;
    result.objectiveValue = objectiveFunction(current);
    result.penaltyValue = currentValue;
    result.equalityValue = equalityConstraint(current);
    result.inequalityValues = inequalityConstraints(current);

    return result;
}

OptimizationResult minimizePenaltyNelderMead(const Point3D &startPoint,
                                             double penaltyParameter,
                                             double tolerance,
                                             int maxIterations,
                                             double initialStep)
{
    OptimizationResult result;
    result.algorithmName = "Penalty Nelder-Mead";
    result.penaltyParameter = penaltyParameter;
    result.startingPoint = startPoint;
    result.solution = startPoint;

    // Initialize simplex: start point + 3 axis-aligned perturbations
    vector<Point3D> simplex(4);
    simplex[0] = startPoint;
    simplex[1] = add(startPoint, {initialStep, 0.0, 0.0});
    simplex[2] = add(startPoint, {0.0, initialStep, 0.0});
    simplex[3] = add(startPoint, {0.0, 0.0, initialStep});

    // Evaluate all vertices
    vector<double> values(4);
    for (int i = 0; i < 4; ++i)
    {
        values[i] = penaltyFunction(simplex[i], penaltyParameter);
        result.functionEvaluations++;
    }
    result.path.push_back(toVector(simplex[0]));

    const double alpha = 1.0; // reflection
    const double beta = 0.5;  // contraction
    const double gamma = 2.0; // expansion

    for (int iteration = 0; iteration < maxIterations; ++iteration)
    {
        // Find worst, second worst, best
        int worst = 0, secondWorst = 1, best = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (values[i] > values[worst])
            {
                secondWorst = worst;
                worst = i;
            }
            else if (values[i] > values[secondWorst] && i != worst)
            {
                secondWorst = i;
            }
            if (values[i] < values[best])
                best = i;
        }

        // Check convergence
        double range = values[worst] - values[best];
        if (range < tolerance)
            break;

        // Centroid of non-worst points
        Point3D centroid = {0.0, 0.0, 0.0};
        for (int i = 0; i < 4; ++i)
            if (i != worst)
                centroid = add(centroid, simplex[i]);
        centroid = scale(centroid, 1.0 / 3.0);

        // Reflect worst point through centroid
        Point3D reflected = add(centroid, scale(subtract(centroid, simplex[worst]), alpha));
        double reflectedValue = penaltyFunction(reflected, penaltyParameter);
        result.functionEvaluations++;

        if (reflectedValue < values[best])
        {
            // Expand beyond reflected point
            Point3D expanded = add(centroid, scale(subtract(reflected, centroid), gamma));
            double expandedValue = penaltyFunction(expanded, penaltyParameter);
            result.functionEvaluations++;

            if (expandedValue < reflectedValue)
            {
                simplex[worst] = expanded;
                values[worst] = expandedValue;
            }
            else
            {
                simplex[worst] = reflected;
                values[worst] = reflectedValue;
            }
        }
        else if (reflectedValue < values[secondWorst])
        {
            // Accept reflection
            simplex[worst] = reflected;
            values[worst] = reflectedValue;
        }
        else
        {
            // Contract toward centroid
            Point3D contracted = add(centroid, scale(subtract(simplex[worst], centroid), beta));
            double contractedValue = penaltyFunction(contracted, penaltyParameter);
            result.functionEvaluations++;

            if (contractedValue < values[worst])
            {
                simplex[worst] = contracted;
                values[worst] = contractedValue;
            }
            else
            {
                // Shrink entire simplex
                for (int i = 0; i < 4; ++i)
                {
                    if (i != best)
                    {
                        simplex[i] = add(simplex[best], scale(subtract(simplex[i], simplex[best]), 0.5));
                        values[i] = penaltyFunction(simplex[i], penaltyParameter);
                        result.functionEvaluations++;
                    }
                }
            }
        }

        result.path.push_back(toVector(simplex[best]));
        result.steps++;
    }

    // Best point is the solution
    int bestIdx = 0;
    for (int i = 1; i < 4; ++i)
        if (values[i] < values[bestIdx])
            bestIdx = i;

    result.solution = simplex[bestIdx];
    result.objectiveValue = objectiveFunction(result.solution);
    result.penaltyValue = values[bestIdx];
    result.equalityValue = equalityConstraint(result.solution);
    result.inequalityValues = inequalityConstraints(result.solution);

    return result;
}
