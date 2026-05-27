# Simplex implementation — what changed and how it works

## What I changed

The repository previously implemented three nonlinear minimization algorithms
(gradient descent, steepest descent, Nelder–Mead) for `f(x,y) = -x·y·(1-x-y)/8`
with an SFML visualization. The new task in [toto.txt](toto.txt) is a **linear
programming problem** that must be solved with a **simplex algorithm**, so I
rewrote the three C++ files in place:

| File | Old contents | New contents |
|---|---|---|
| [lib.hpp](lib.hpp) | objective `f`, gradient, `OptimizationResult`, SFML headers | `LPProblem`, `SimplexStep`/`SimplexResult`, declarations of simplex + printers + `compareResults` + `visualizeLP` |
| [algoritmai.cpp](algoritmai.cpp) | `gradientDescent`, `steepestDescent`, `nelderMead` | `solveSimplex` + tableau / problem printers + `compareResults` |
| [main.cpp](main.cpp) | drove the three algorithms from three start points and visualized | defines both LPs (official + individual), solves both, compares, prompts for visualization |
| [vizualizacija.cpp](vizualizacija.cpp) | *(did not exist; visualization was inlined in `main.cpp`)* | `visualizeLP(...)` — SFML side-by-side plot of the (x₂, x₄) projection for both problems with step navigation |
| [CMakeLists.txt](CMakeLists.txt) | linked SFML for the nonlinear plot | links SFML again for `vizualizacija.cpp`; lists three source files |

## The problem

From [toto.txt](toto.txt), the **official task**:

```
min  2 x1 - 3 x2 - 5 x4
s.t. -x1 + x2 - x3 - x4 <= 8
      2 x1 + 4 x2       <= 10
                x3 + x4 <= 3
      xi >= 0
```

The **individual task** keeps the same objective and constraint matrix but
replaces the right-hand side `(8, 10, 3)` with the student-ID digits
`(a, b, c) = (9, 3, 7)`. The lab requires solving both and comparing the
results (minimum objective value, optimal solution, optimal basis).

### Standard form

Introduce non-negative slack variables `x5, x6, x7` to turn each `<=` into
`=`:

```
-x1 + x2 - x3 - x4 + x5           = b1
 2x1 + 4x2                  + x6  = b2
            x3 + x4              + x7 = b3
```

In matrix form: `min cᵀx  s.t.  [A | I] x' = b,  x' >= 0`, where
`x' = (x1..x4, x5..x7)` and `c = (2, -3, 0, -5, 0, 0, 0)`.

## Execution flow

Below is what happens when you run [optimization_methods.exe](optimization_methods.exe),
function by function.

### 1. `main()` — [main.cpp:8](main.cpp#L8)

1. Sets `cout` to `fixed` with 4-decimal precision.
2. Prints the student-ID digits `a, b, c` declared at file scope
   ([main.cpp:3-5](main.cpp#L3-L5)).
3. Builds the **official task** `LPProblem`:
   - `c  = {2, -3, 0, -5}`
   - `A  = {{-1,1,-1,-1}, {2,4,0,0}, {0,0,1,1}}`
   - `b  = {8, 10, 3}`
4. Calls `printProblemMatrixForm(official)` then
   `printStandardForm(official)` (see §2 and §3 below).
5. Calls `solveSimplex(official, true)` (§4). The `true` makes it print
   every pivot and tableau.
6. Calls `printResult(...)` (§5) to summarize.
7. Repeats steps 3–6 for the **individual task** — a copy of `official`
   with `b = {a, b, c} = {9, 3, 7}`.
8. Calls `compareResults(...)` (§6) to print the side-by-side table the lab
   asks for.
9. Prompts to visualize and, on `y`, calls `visualizeLP(...)` (§7).

### 2. `printProblemMatrixForm(lp)` — [algoritmai.cpp:58](algoritmai.cpp#L58)

Pretty-prints `cᵀ`, `A`, and `b` as matrices. Just an information block — no
math is done here.

### 3. `printStandardForm(lp)` — [algoritmai.cpp:87](algoritmai.cpp#L87)

Writes the LP in standard form. Internally calls `printTerm` (anonymous
namespace, [algoritmai.cpp:40](algoritmai.cpp#L40)) for each `x_j` so the
output reads like algebra (skip zero coefficients, drop the `1` from `±1·x_j`,
choose `+`/`-` correctly). After listing the constraints with slack variables,
it prints the objective.

### 4. `solveSimplex(lp, verbose)` — [algoritmai.cpp:109](algoritmai.cpp#L109)

This is the simplex algorithm itself. The convention used:

- **Tableau layout:** `(m+1) × (n+m+1)`.
  - Row 0 is the **z-row**: stores `z_j - c_j` for every variable plus the
    current objective value `z = c_Bᵀ x_B` in the last column.
  - Rows `1..m` are the constraint rows `[A | I | b]`.
- **Optimality:** every entry of row 0 (except the RHS) is `<= 0`.
- **Entering variable:** column with the **most positive** `z_j - c_j`.
- **Leaving variable:** **minimum-ratio test** on positive entries of the
  pivot column.

Step by step:

1. **Build the extended cost vector** `c_ext` of length `n + m`, with zeros
   appended for the slacks ([algoritmai.cpp:116](algoritmai.cpp#L116)).
2. **Initialize the tableau:**
   - Row 0 starts as `-c_ext` (because the initial basis is the slacks with
     `c_B = 0`, so `z_j - c_j = -c_j`).
   - Constraint rows are filled with `A`, an identity block (for the slacks),
     and `b` ([algoritmai.cpp:124-135](algoritmai.cpp#L124-L135)).
3. **Initialize the basis** to the slack indices `{n, n+1, ..., n+m-1}`
   ([algoritmai.cpp:138-140](algoritmai.cpp#L138-L140)).
4. **Print the initial tableau** via the local `printTableau` helper
   ([algoritmai.cpp:7](algoritmai.cpp#L7)) if `verbose`.
5. **Main loop** ([algoritmai.cpp:148](algoritmai.cpp#L148)), up to
   `MAX_ITER = 1000`:
   1. Scan row 0 for the **most positive** entry → `pivotCol`. If none
      exists (all `<= EPS`), set `result.optimal = true` and break.
   2. Run the **min-ratio test**: among rows where `tab[i][pivotCol] > EPS`,
      pick the one minimizing `tab[i][RHS] / tab[i][pivotCol]` → `pivotRow`.
      If no row has a positive entry, the problem is **unbounded** and we
      return early.
   3. Log the pivot (`x_j enters`, `x_k leaves`) if `verbose`.
   4. **Pivot operation:**
      - Divide `tab[pivotRow]` by the pivot element so it becomes 1.
      - For every other row, subtract `factor × pivotRow` so the pivot
        column becomes a unit vector.
      - Update `basis[pivotRow-1] = pivotCol`.
   5. Print the updated tableau if `verbose`.
6. **Build the result** ([algoritmai.cpp:208](algoritmai.cpp#L208)):
   - `solution_all[basis[i]] = tab[i+1][RHS]` for each basic row; non-basic
     variables stay at 0.
   - Split into `x` (decision variables, first `n`) and `slack` (last `m`).
   - `optimalValue = tab[0][RHS]` (which equals the current `z`).
7. Return the `SimplexResult`.

### 5. `printResult(res, n, m)` — [algoritmai.cpp:223](algoritmai.cpp#L223)

Prints iterations, optimal `z`, every `x_j` and slack `x_{n+i}`, and the
final basis as a set like `{x5, x2, x4}`.

### 6. `compareResults(r1, name1, r2, name2, n, m)` — [algoritmai.cpp:280](algoritmai.cpp#L280)

Prints the side-by-side comparison table the lab asks for: optimal
objective value, iteration count, every decision variable, every slack, and
the optimal basis — one column for the official task, one for the
individual.

### 7. `visualizeLP(lp1, r1, lp2, r2)` — [vizualizacija.cpp](vizualizacija.cpp)

After all the text output, `main()` prompts:

```
Visualize both problems in a window? (y/n):
```

If you answer `y`, `visualizeLP` opens a single 1340×800 SFML window split
into two panels — **official on the left, individual on the right** — so the
two pivot paths can be compared directly.

**The projection.** Each LP has 4 decision variables — too many to plot
directly. Both optima have `x1 = x3 = 0`, and that holds along the entire
pivot path (`solveSimplex` records `result.steps` after the initial tableau
and after every pivot, so we know the full sequence of vertices). Projecting
to **(x₂, x₄)** therefore loses nothing. With `x1 = x3 = 0` the constraints
reduce to:

```
x2 - x4 <= b1        (first constraint)
4 x2    <= b2   →   x2 <= b2 / 4
x4      <= b3
x2, x4  >= 0
```

`b1` is large enough in both problems that the first constraint never binds,
so each projected feasible region is a rectangle `[0, b2/4] × [0, b3]`
(official: `[0, 2.5] × [0, 3]`, individual: `[0, 0.75] × [0, 7]`).

**Step-by-step navigation.** `solveSimplex` stores `res.steps` — one
`SimplexStep` for the initial vertex and one for each pivot. Each step
records the full solution vector, the basis, the current `z`, and which
variable entered / left. The window starts at step 0; the arrow keys drive
**both panels in lockstep** (they happen to take the same number of steps):

| Key | Action |
|---|---|
| `→` | Next step (next pivot) |
| `←` | Previous step |
| `Home` | Jump to step 0 |
| `End` | Jump to the optimal step |
| `Esc` / close button | Exit |

The window title shows `Step N / M` and only the portion of each path
reached so far is drawn. The currently selected vertex is highlighted with a
yellow ring.

**What each panel draws** (`drawPanel`, [vizualizacija.cpp:73](vizualizacija.cpp#L73)):

1. The **problem name** above the plot.
2. **Heatmap** of `z = -3·x₂ - 5·x₄` inside the feasible set. Lighter =
   higher z (closer to 0); darker = lower z (the optimum corner).
3. **Outline** of the feasible rectangle.
4. **Axes + ticks + labels** at `0`, `b2/8`, `b2/4` on the x-axis and `0`,
   `b3/2`, `b3` on the y-axis.
5. **Partial pivot path**: red line segments with arrowheads connecting the
   first `currentStep` vertex projections. Start = hollow green square,
   optimum = filled red circle, current step = yellow ring.
6. **HUD** beneath the plot showing the *current step's* state: step number,
   pivot info (`x_j enters, x_k leaves`) or "initial vertex", `z`, all `xⱼ`
   and slack values, the basis, and the `b` vector.

**Main loop** (`visualizeLP`, end of [vizualizacija.cpp](vizualizacija.cpp))
keeps `currentStep` as state between frames. It polls events (arrow keys
update `currentStep`, `Esc` / close exits), updates the window title, then
`clear → drawPanel(left) → drawPanel(right) → separator → display`. Capped
at 60 FPS via `setFramerateLimit(60)`.

## Trace of the official task

Converges in exactly **two iterations**. Trace produced by the verbose run.

### Initial tableau

```
            x1     x2     x3     x4     x5     x6     x7    RHS
  z   |   -2      3      0      5      0      0      0      0
  x5  |  -1      1     -1     -1      1      0      0      8
  x6  |   2      4      0      0      0      1      0     10
  x7  |   0      0      1      1      0      0      1      3
```

Row 0 = `-c`. Most positive entry is `5` under `x4` → `x4` enters. Only row
`x7` has a positive entry in that column, so `x7` leaves; ratio `3/1 = 3`.

### After iteration 1 (x4 in, x7 out)

```
            x1     x2     x3     x4     x5     x6     x7    RHS
  z   |   -2      3     -5      0      0      0     -5    -15
  x5  |  -1      1      0      0      1      0      1     11
  x6  |   2      4      0      0      0      1      0     10
  x4  |   0      0      1      1      0      0      1      3
```

Most positive in row 0 is `3` under `x2` → `x2` enters. Min-ratio: row `x6`
(`10/4 = 2.5`) beats row `x5` (`11/1 = 11`), so `x6` leaves.

### After iteration 2 (x2 in, x6 out)

```
            x1     x2     x3     x4     x5     x6     x7     RHS
  z   |  -3.5    0     -5      0      0    -0.75   -5     -22.5
  x5  |  -1.5   0      0      0      1    -0.25    1      8.5
  x2  |   0.5   1      0      0      0     0.25    0      2.5
  x4  |   0     0      1      1      0      0      1      3
```

Row 0 has no positive entry → optimal.

**Solution:** `x* = (0, 2.5, 0, 3)`, `z_min = -22.5`, basis = `{x5, x2, x4}`.
Sanity check: `2·0 - 3·2.5 - 5·3 = -22.5`. ✓

## Trace of the individual task

Same pivot sequence (`x4` then `x2`), with `b = (9, 3, 7)`.

### Initial tableau

```
            x1     x2     x3     x4     x5     x6     x7    RHS
  z   |   -2      3      0      5      0      0      0      0
  x5  |  -1      1     -1     -1      1      0      0      9
  x6  |   2      4      0      0      0      1      0      3
  x7  |   0      0      1      1      0      0      1      7
```

Row 0 = `-c`. Most positive entry is `5` in column `x4`, so `x4` enters.
Only row `x7` has a positive entry in column `x4` (it is `1`), so the
min-ratio test picks `x7` as the leaving variable.

### After iteration 1 (x4 in, x7 out)

```
            x1     x2     x3     x4     x5     x6     x7    RHS
  z   |   -2      3     -5      0      0      0     -5    -35
  x5  |  -1      1      0      0      1      0      1     16
  x6  |   2      4      0      0      0      1      0      3
  x4  |   0      0      1      1      0      0      1      7
```

Current basis `{x5, x6, x4}`, `z = -35`. Most positive entry in row 0 is
`3` under `x2`. Min-ratio: row `x6` (3 / 4 = 0.75) wins over row `x5`
(16 / 1 = 16).

### After iteration 2 (x2 in, x6 out)

```
            x1     x2     x3     x4     x5     x6     x7      RHS
  z   |  -3.5    0     -5      0      0    -0.75   -5      -37.25
  x5  |  -1.5   0      0      0      1    -0.25    1       15.25
  x2  |   0.5   1      0      0      0     0.25    0        0.75
  x4  |   0     0      1      1      0      0      1        7
```

Row 0 now has no positive entry → optimal.

**Solution:** `x* = (0, 0.75, 0, 7)`, `z_min = -37.25`, basis = `{x5, x2, x4}`.

Sanity check: `2·0 - 3·0.75 - 5·7 = 0 - 2.25 - 35 = -37.25`. ✓

## Comparison

This is the table the lab asks for (`compareResults` prints the same thing):

| Quantity | Official `(8,10,3)` | Individual `(9,3,7)` |
|---|---|---|
| Optimal `z` | **−22.5** | **−37.25** |
| `x1` | 0 | 0 |
| `x2` | 2.5 | 0.75 |
| `x3` | 0 | 0 |
| `x4` | 3 | 7 |
| slack `x5` | 8.5 | 15.25 |
| slack `x6` | 0 | 0 |
| slack `x7` | 0 | 0 |
| Basis | `{x5, x2, x4}` | `{x5, x2, x4}` |
| Iterations | 2 | 2 |

**Interpretation.** Both problems land on the **same optimal basis**
`{x5, x2, x4}`. Changing only the right-hand side shifts the polytope but
keeps the *structure* of which constraints are tight at the optimum:
constraints 2 (`2x1 + 4x2 ≤ b2`) and 3 (`x3 + x4 ≤ b3`) bind in both cases —
fixing `x2 = b2/4` and `x4 = b3` — while `x1 = x3 = 0` because their reduced
costs stay positive. The first constraint stays slack in both (`x5 > 0`).
Only the numeric values of the solution and the objective differ.

## Build

Direct compile with `g++` (the line I actually used):

```powershell
g++ -std=c++17 -O2 main.cpp algoritmai.cpp vizualizacija.cpp -o optimization_methods.exe -lsfml-graphics -lsfml-window -lsfml-system
```

Breakdown of every flag:

| Token | Meaning |
|---|---|
| `g++` | GNU C++ compiler driver |
| `-std=c++17` | Use the C++17 language standard (needed for `constexpr`, `auto` lambdas, structured init in `LPProblem`, etc.) |
| `-O2` | Enable level-2 optimizations (inlining, loop opts) — the LP is tiny but this costs nothing |
| `main.cpp` | First translation unit — defines `main()` and the two problems |
| `algoritmai.cpp` | Second translation unit — defines `solveSimplex` and the printers |
| `vizualizacija.cpp` | Third translation unit — defines `visualizeLP` (SFML window) |
| `-o optimization_methods.exe` | Name of the output executable |
| `-lsfml-graphics` | Link the SFML graphics library (shapes, text, window rendering) |
| `-lsfml-window` | Link the SFML window/event library |
| `-lsfml-system` | Link the SFML system library (math, time, threading primitives) |

A more verbose variant with extra warnings (useful while developing):

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic main.cpp algoritmai.cpp vizualizacija.cpp -o optimization_methods.exe -lsfml-graphics -lsfml-window -lsfml-system
```

Then run:

```powershell
./optimization_methods.exe
```

CMake also still works (no SFML required anymore):

```powershell
cmake -S . -B build
cmake --build build --config Release
```

CMake internally invokes the same `g++` (or MSVC `cl.exe`, depending on your
generator) with the C++17 standard from
[CMakeLists.txt](CMakeLists.txt#L4-L5) and produces
`build/optimization_methods.exe` (or `build/Release/optimization_methods.exe`
with the Visual Studio generator).
