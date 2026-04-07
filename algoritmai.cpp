#include "lib.hpp"

// Algorithm 6: Gradient Descent
OptimizationResult gradientDescent(double startX, double startY, double learningRate = 0.01,
                                   double tolerance = 1e-6, int maxIterations = 10000)
{
    OptimizationResult result;
    result.algorithmName = "Gradient Descent";
    result.startingX = startX;
    result.startingY = startY;
    result.steps = 0;
    result.functionEvaluations = 0;

    double x = startX, y = startY;

    // Store initial path point
    result.path.push_back({x, y});
    result.functionEvaluations++;

    for (int i = 0; i < maxIterations; i++)
    {
        double gradX, gradY;
        computeGradient(x, y, gradX, gradY);

        double gradMag = sqrt(gradX * gradX + gradY * gradY);
        if (gradMag < tolerance)
        {
            break;
        }

        // Update step
        double newX = x - learningRate * gradX;
        double newY = y - learningRate * gradY;

        // Ensure we stay in valid region
        if (!isValidPoint(newX, newY))
        {
            break;
        }

        x = newX;
        y = newY;
        result.path.push_back({x, y});
        result.functionEvaluations++;
        result.steps++;
    }

    result.solution = {x, y};
    result.minValue = objectiveFunction(x, y);

    return result;
}

// Algorithm 7: Steepest Descent (with line search)
OptimizationResult steepestDescent(double startX, double startY, double tolerance = 1e-6,
                                   int maxIterations = 10000)
{
    OptimizationResult result;
    result.algorithmName = "Steepest Descent";
    result.startingX = startX;
    result.startingY = startY;
    result.steps = 0;
    result.functionEvaluations = 0;

    double x = startX, y = startY;

    // Store initial path point
    result.path.push_back({x, y});
    result.functionEvaluations++;

    for (int i = 0; i < maxIterations; i++)
    {
        double gradX, gradY;
        computeGradient(x, y, gradX, gradY);

        double gradMag = sqrt(gradX * gradX + gradY * gradY);
        if (gradMag < tolerance)
        {
            break;
        }

        // Line search: find best step size
        double stepSize = 0.1;
        double bestStepSize = stepSize;
        double bestValue = objectiveFunction(x - stepSize * gradX, y - stepSize * gradY);
        result.functionEvaluations++;

        for (int j = 0; j < 10; j++)
        {
            stepSize *= 0.5;
            double testX = x - stepSize * gradX;
            double testY = y - stepSize * gradY;

            if (!isValidPoint(testX, testY))
                break;

            double testValue = objectiveFunction(testX, testY);
            result.functionEvaluations++;

            if (testValue < bestValue)
            {
                bestValue = testValue;
                bestStepSize = stepSize;
            }
        }

        double newX = x - bestStepSize * gradX;
        double newY = y - bestStepSize * gradY;

        if (!isValidPoint(newX, newY))
        {
            break;
        }

        x = newX;
        y = newY;
        result.path.push_back({x, y});
        result.steps++;
    }

    result.solution = {x, y};
    result.minValue = objectiveFunction(x, y);

    return result;
}

// Algorithm 8: Nelder-Mead (Simplex)
OptimizationResult nelderMead(double startX, double startY, double tolerance = 1e-6,
                              int maxIterations = 10000)
{
    OptimizationResult result;
    result.algorithmName = "Nelder-Mead (Simplex)";
    result.startingX = startX;
    result.startingY = startY;
    result.steps = 0;
    result.functionEvaluations = 0;

    // Standard Nelder-Mead constants
    const double alpha = 1.0; // reflection
    const double gamma = 2.0; // expansion
    const double rho = 0.5;   // contraction
    const double sigma = 0.5; // shrink

    auto projectToFeasible = [](double x, double y) -> vector<double>
    {
        x = max(0.0, x);
        y = max(0.0, y);
        if (x + y > 1.0)
        {
            double sum = x + y;
            if (sum > 0.0)
            {
                x /= sum;
                y /= sum;
            }
        }
        return {x, y};
    };

    auto eval = [&](const vector<double> &p) -> double
    {
        result.functionEvaluations++;
        return objectiveFunction(p[0], p[1]);
    };

    const double initialStep = 0.1;
    vector<double> p0 = projectToFeasible(startX, startY);
    vector<double> p1 = projectToFeasible(startX + initialStep, startY);
    vector<double> p2 = projectToFeasible(startX, startY + initialStep);

    vector<vector<double>> vertices = {p0, p1, p2};
    vector<double> values = {eval(vertices[0]), eval(vertices[1]), eval(vertices[2])};

    result.path.push_back(vertices[0]);

    for (int iter = 0; iter < maxIterations; iter++)
    {
        vector<int> order = {0, 1, 2};
        sort(order.begin(), order.end(), [&](int lhs, int rhs)
             { return values[lhs] < values[rhs]; });

        int best = order[0];
        int mid = order[1];
        int worst = order[2];

        double fBest = values[best];
        double fMid = values[mid];
        double fWorst = values[worst];

        double simplexSize = max(
            distance(vertices[best][0], vertices[best][1], vertices[mid][0], vertices[mid][1]),
            max(
                distance(vertices[best][0], vertices[best][1], vertices[worst][0], vertices[worst][1]),
                distance(vertices[mid][0], vertices[mid][1], vertices[worst][0], vertices[worst][1])));

        if ((fWorst - fBest) < tolerance && simplexSize < tolerance)
        {
            break;
        }

        // Centroid of the best two points
        vector<double> c = {
            (vertices[best][0] + vertices[mid][0]) / 2.0,
            (vertices[best][1] + vertices[mid][1]) / 2.0};

        // Reflection
        vector<double> r = projectToFeasible(
            c[0] + alpha * (c[0] - vertices[worst][0]),
            c[1] + alpha * (c[1] - vertices[worst][1]));
        double fR = eval(r);

        bool accepted = false;

        if (fR < fBest)
        {
            // Expansion
            vector<double> e = projectToFeasible(
                c[0] + gamma * (r[0] - c[0]),
                c[1] + gamma * (r[1] - c[1]));
            double fE = eval(e);
            if (fE < fR)
            {
                vertices[worst] = e;
                values[worst] = fE;
            }
            else
            {
                vertices[worst] = r;
                values[worst] = fR;
            }
            accepted = true;
        }
        else if (fR < fMid)
        {
            vertices[worst] = r;
            values[worst] = fR;
            accepted = true;
        }

        if (!accepted)
        {
            // Contraction (outside or inside)
            vector<double> contracted;
            if (fR < fWorst)
            {
                contracted = projectToFeasible(
                    c[0] + rho * (r[0] - c[0]),
                    c[1] + rho * (r[1] - c[1]));
            }
            else
            {
                contracted = projectToFeasible(
                    c[0] + rho * (vertices[worst][0] - c[0]),
                    c[1] + rho * (vertices[worst][1] - c[1]));
            }

            double fC = eval(contracted);
            if (fC < fWorst)
            {
                vertices[worst] = contracted;
                values[worst] = fC;
            }
            else
            {
                // Shrink simplex towards best
                for (int idx : order)
                {
                    if (idx == best)
                    {
                        continue;
                    }
                    vertices[idx] = projectToFeasible(
                        vertices[best][0] + sigma * (vertices[idx][0] - vertices[best][0]),
                        vertices[best][1] + sigma * (vertices[idx][1] - vertices[best][1]));
                    values[idx] = eval(vertices[idx]);
                }
            }
        }

        result.path.push_back(vertices[best]);
        result.steps++;
    }

    int bestIdx = 0;
    for (int i = 1; i < 3; i++)
    {
        if (values[i] < values[bestIdx])
        {
            bestIdx = i;
        }
    }

    result.solution = vertices[bestIdx];
    result.minValue = values[bestIdx];

    return result;
}
