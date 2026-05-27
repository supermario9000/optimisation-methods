#ifndef LIB_HPP
#define LIB_HPP

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>

using namespace std;

// Student ID digits from "2x1xabc" -- used as RHS in the individual task.
extern int a;
extern int b;
extern int c;

using Vector = vector<double>;
using Matrix = vector<vector<double>>;

// Linear programming problem in the form:
//     min c^T x   s.t.   A x <= b,   x >= 0
struct LPProblem {
    Vector c;
    Matrix A;
    Vector b;
    string name;
};

struct SimplexStep {
    Vector point;          // full solution vector (length n+m)
    vector<int> basis;     // basis indices after this step
    double z = 0.0;        // current objective value
    int enteringVar = -1;  // -1 for the initial step
    int leavingVar = -1;
};

struct SimplexResult {
    bool optimal = false;
    bool unbounded = false;
    Vector x;             // decision variables (length n)
    Vector slack;         // slack variables   (length m)
    Vector solution_all;  // x and slack combined
    vector<int> basis;    // indices into [0, n+m) of basic variables
    double optimalValue = 0.0;
    int iterations = 0;
    vector<SimplexStep> steps;  // initial + after every pivot
};

SimplexResult solveSimplex(const LPProblem &lp, bool verbose = true);

void printProblemMatrixForm(const LPProblem &lp);
void printStandardForm(const LPProblem &lp);
void printResult(const SimplexResult &res, int n_orig, int m);
void compareResults(const SimplexResult &r1, const string &name1,
                    const SimplexResult &r2, const string &name2,
                    int n_orig, int m);

void visualizeLP(const LPProblem &lp1, const SimplexResult &r1,
                 const LPProblem &lp2, const SimplexResult &r2);

#endif
