#include "lib.hpp"

namespace {
constexpr double EPS = 1e-9;

void printTableau(const Matrix &tab, const vector<int> &basis, int total)
{
    int rows = static_cast<int>(tab.size());
    int cols = static_cast<int>(tab[0].size());

    cout << "        ";
    for (int j = 0; j < total; j++)
    {
        string name = "x" + to_string(j + 1);
        cout << setw(10) << name;
    }
    cout << setw(10) << "RHS" << "\n";

    cout << "  z   | ";
    for (int j = 0; j < cols; j++)
        cout << setw(10) << tab[0][j];
    cout << "\n";

    for (int i = 1; i < rows; i++)
    {
        string name = "x" + to_string(basis[i - 1] + 1);
        cout << "  " << setw(4) << name << "| ";
        for (int j = 0; j < cols; j++)
            cout << setw(10) << tab[i][j];
        cout << "\n";
    }
    cout << "\n";
}

void printTerm(double v, int idx, bool &first)
{
    if (fabs(v) < EPS)
        return;
    if (first)
    {
        if (v < 0)
            cout << "-";
        first = false;
    }
    else
    {
        cout << (v >= 0 ? " + " : " - ");
    }
    double mag = fabs(v);
    if (fabs(mag - 1.0) > EPS)
        cout << mag;
    cout << "x" << (idx + 1);
}
} // namespace

void printProblemMatrixForm(const LPProblem &lp)
{
    int n = static_cast<int>(lp.c.size());
    int m = static_cast<int>(lp.b.size());

    cout << "=== " << lp.name << ": matrix form ===\n";
    cout << "min  c^T x    s.t.  A x <= b,   x >= 0\n\n";

    cout << "c^T = [ ";
    for (int j = 0; j < n; j++)
        cout << setw(8) << lp.c[j] << " ";
    cout << "]\n";

    cout << "A   =\n";
    for (int i = 0; i < m; i++)
    {
        cout << "        [ ";
        for (int j = 0; j < n; j++)
            cout << setw(8) << lp.A[i][j] << " ";
        cout << "]\n";
    }

    cout << "b   = [ ";
    for (int i = 0; i < m; i++)
        cout << setw(8) << lp.b[i] << " ";
    cout << "]^T\n\n";
}

void printStandardForm(const LPProblem &lp)
{
    int n = static_cast<int>(lp.c.size());
    int m = static_cast<int>(lp.b.size());

    cout << "=== " << lp.name << ": standard form ===\n";
    cout << "Slack variables x" << (n + 1) << " ... x" << (n + m) << " >= 0:\n";
    for (int i = 0; i < m; i++)
    {
        cout << "  ";
        bool first = true;
        for (int j = 0; j < n; j++)
            printTerm(lp.A[i][j], j, first);
        printTerm(1.0, n + i, first);
        cout << " = " << lp.b[i] << "\n";
    }
    cout << "  min ";
    bool first = true;
    for (int j = 0; j < n; j++)
        printTerm(lp.c[j], j, first);
    cout << "\n\n";
}

SimplexResult solveSimplex(const LPProblem &lp, bool verbose)
{
    SimplexResult result;

    int n = static_cast<int>(lp.c.size());
    int m = static_cast<int>(lp.b.size());
    int total = n + m;

    // c with zeros appended for the slack variables.
    Vector c_ext(total, 0.0);
    for (int j = 0; j < n; j++)
        c_ext[j] = lp.c[j];

    // Tableau:
    //   row 0          : [ z_j - c_j  |  current z ]
    //   rows 1..m      : [ A | I | b ]
    // For minimization, optimality is reached when row 0 has no strictly
    // positive entry; the most positive entry chooses the entering variable.
    Matrix tab(m + 1, Vector(total + 1, 0.0));
    for (int j = 0; j < total; j++)
        tab[0][j] = -c_ext[j];
    tab[0][total] = 0.0;

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            tab[i + 1][j] = lp.A[i][j];
        tab[i + 1][n + i] = 1.0;
        tab[i + 1][total] = lp.b[i];
    }

    // Initial basis: the slack variables.
    vector<int> basis(m);
    for (int i = 0; i < m; i++)
        basis[i] = n + i;

    auto recordStep = [&](int entering, int leaving) {
        SimplexStep s;
        s.point.assign(total, 0.0);
        for (int i = 0; i < m; i++)
            s.point[basis[i]] = tab[i + 1][total];
        s.basis = basis;
        s.z = tab[0][total];
        s.enteringVar = entering;
        s.leavingVar = leaving;
        result.steps.push_back(s);
    };
    recordStep(-1, -1);

    if (verbose)
    {
        cout << "Initial tableau:\n";
        printTableau(tab, basis, total);
    }

    const int MAX_ITER = 1000;
    int iter = 0;
    for (; iter < MAX_ITER; iter++)
    {
        // Entering variable: most positive z_j - c_j in row 0.
        int pivotCol = -1;
        double bestVal = EPS;
        for (int j = 0; j < total; j++)
        {
            if (tab[0][j] > bestVal)
            {
                bestVal = tab[0][j];
                pivotCol = j;
            }
        }

        if (pivotCol == -1)
        {
            result.optimal = true;
            break;
        }

        // Leaving variable: minimum-ratio test on positive entries of pivot column.
        int pivotRow = -1;
        double bestRatio = numeric_limits<double>::infinity();
        for (int i = 1; i <= m; i++)
        {
            if (tab[i][pivotCol] > EPS)
            {
                double ratio = tab[i][total] / tab[i][pivotCol];
                if (ratio < bestRatio - EPS)
                {
                    bestRatio = ratio;
                    pivotRow = i;
                }
            }
        }

        if (pivotRow == -1)
        {
            result.unbounded = true;
            result.iterations = iter;
            if (verbose)
                cout << "Problem is UNBOUNDED.\n";
            return result;
        }

        if (verbose)
        {
            cout << "Iteration " << (iter + 1)
                 << ": x" << (pivotCol + 1) << " enters, x"
                 << (basis[pivotRow - 1] + 1) << " leaves"
                 << " (pivot = " << tab[pivotRow][pivotCol] << ")\n";
        }

        double piv = tab[pivotRow][pivotCol];
        for (int j = 0; j <= total; j++)
            tab[pivotRow][j] /= piv;
        for (int i = 0; i <= m; i++)
        {
            if (i == pivotRow)
                continue;
            double factor = tab[i][pivotCol];
            if (fabs(factor) < EPS)
                continue;
            for (int j = 0; j <= total; j++)
                tab[i][j] -= factor * tab[pivotRow][j];
        }
        int leavingVar = basis[pivotRow - 1];
        basis[pivotRow - 1] = pivotCol;
        recordStep(pivotCol, leavingVar);

        if (verbose)
            printTableau(tab, basis, total);
    }

    if (!result.optimal && !result.unbounded)
    {
        if (verbose)
            cout << "Maximum iterations (" << MAX_ITER << ") reached.\n";
    }

    result.iterations = iter;
    result.solution_all.assign(total, 0.0);
    for (int i = 0; i < m; i++)
        result.solution_all[basis[i]] = tab[i + 1][total];
    result.x.assign(result.solution_all.begin(), result.solution_all.begin() + n);
    result.slack.assign(result.solution_all.begin() + n, result.solution_all.end());
    result.basis = basis;
    result.optimalValue = tab[0][total];
    return result;
}

void printResult(const SimplexResult &res, int n_orig, int m)
{
    if (res.unbounded)
    {
        cout << "Problem is UNBOUNDED.\n";
        return;
    }
    if (!res.optimal)
    {
        cout << "Did not converge.\n";
        return;
    }

    cout << "Iterations: " << res.iterations << "\n";
    cout << "Optimal objective value: " << res.optimalValue << "\n";
    cout << "Decision variables:\n";
    for (int j = 0; j < n_orig; j++)
        cout << "  x" << (j + 1) << " = " << res.x[j] << "\n";
    cout << "Slack variables:\n";
    for (int i = 0; i < m; i++)
        cout << "  x" << (n_orig + i + 1) << " = " << res.slack[i] << "\n";
    cout << "Optimal basis: { ";
    for (size_t i = 0; i < res.basis.size(); i++)
    {
        cout << "x" << (res.basis[i] + 1);
        if (i + 1 < res.basis.size())
            cout << ", ";
    }
    cout << " }\n";
}

void compareResults(const SimplexResult &r1, const string &name1,
                    const SimplexResult &r2, const string &name2,
                    int n_orig, int m)
{
    auto basisStr = [](const SimplexResult &r) {
        string s = "{";
        for (size_t i = 0; i < r.basis.size(); i++)
        {
            s += "x" + to_string(r.basis[i] + 1);
            if (i + 1 < r.basis.size())
                s += ",";
        }
        s += "}";
        return s;
    };

    cout << "=== Comparison ===\n";
    cout << left << setw(28) << "Quantity"
         << setw(22) << name1
         << setw(22) << name2 << "\n";
    cout << string(72, '-') << "\n";

    cout << left << setw(28) << "Optimal objective value"
         << setw(22) << r1.optimalValue
         << setw(22) << r2.optimalValue << "\n";

    cout << left << setw(28) << "Iterations"
         << setw(22) << r1.iterations
         << setw(22) << r2.iterations << "\n";

    for (int j = 0; j < n_orig; j++)
    {
        cout << left << setw(28) << ("x" + to_string(j + 1))
             << setw(22) << r1.x[j]
             << setw(22) << r2.x[j] << "\n";
    }
    for (int i = 0; i < m; i++)
    {
        cout << left << setw(28) << ("slack x" + to_string(n_orig + i + 1))
             << setw(22) << r1.slack[i]
             << setw(22) << r2.slack[i] << "\n";
    }
    cout << left << setw(28) << "Optimal basis"
         << setw(22) << basisStr(r1)
         << setw(22) << basisStr(r2) << "\n\n";
}
