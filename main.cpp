#include "lib.hpp"

// Student ID digits from "2x1xabc": last three digits a, b, c.
int a = 9;
int b = 3;
int c = 7;

int main()
{
    cout << fixed << setprecision(4);
    cout << "=== Linear Programming Laboratory (Simplex method) ===\n";
    cout << "Student ID digits: a = " << a
         << ", b = " << b
         << ", c = " << c << "\n\n";

    // Official task:
    //   min  2 x1 - 3 x2 + 0 x3 - 5 x4
    //   s.t. -x1 + x2 - x3 - x4 <= 8
    //         2 x1 + 4 x2       <= 10
    //                  x3 + x4  <= 3
    //         xi >= 0
    LPProblem official;
    official.name = "Official task";
    official.c = {2.0, -3.0, 0.0, -5.0};
    official.A = {
        {-1.0, 1.0, -1.0, -1.0},
        { 2.0, 4.0,  0.0,  0.0},
        { 0.0, 0.0,  1.0,  1.0}};
    official.b = {8.0, 10.0, 3.0};

    printProblemMatrixForm(official);
    printStandardForm(official);
    cout << "--- Solving official task ---\n";
    SimplexResult resOfficial = solveSimplex(official, true);
    cout << "--- Official task result ---\n";
    printResult(resOfficial,
                static_cast<int>(official.c.size()),
                static_cast<int>(official.b.size()));
    cout << "\n";

    // Individual task: same A and c, RHS replaced with the student-ID digits.
    LPProblem individual = official;
    individual.name = "Individual task";
    individual.b = {static_cast<double>(a),
                    static_cast<double>(b),
                    static_cast<double>(c)};

    printProblemMatrixForm(individual);
    printStandardForm(individual);
    cout << "--- Solving individual task ---\n";
    SimplexResult resIndividual = solveSimplex(individual, true);
    cout << "--- Individual task result ---\n";
    printResult(resIndividual,
                static_cast<int>(individual.c.size()),
                static_cast<int>(individual.b.size()));
    cout << "\n";

    compareResults(resOfficial, "Official",
                   resIndividual, "Individual",
                   static_cast<int>(official.c.size()),
                   static_cast<int>(official.b.size()));

    cout << "Visualize both problems in a window? (y/n): ";
    char response = 0;
    cin >> response;
    if (response == 'y' || response == 'Y')
    {
        visualizeLP(official, resOfficial, individual, resIndividual);
    }
    else
    {
        cout << "Skipping visualization.\n";
    }

    return 0;
}
