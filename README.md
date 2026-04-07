# Optimization Methods Laboratory

A C++ project implementing three optimization algorithms to solve a box volume optimization problem.

## Project Overview

This project solves the optimization problem of maximizing volume per unit surface area of a rectangular box.

**Objective Function:** f(x, y) = -x*y*(1-x-y)/8

**Constraint:** x + y ≤ 1 (where x, y ≥ 0)

## Algorithms Implemented

1. **Gradient Descent** (algoritmai.cpp)
   - Uses fixed learning rate
   - Simple and fast convergence

2. **Steepest Descent** (algoritmai.cpp)
   - Includes line search for optimal step size
   - More refined convergence

3. **Nelder-Mead (Simplex)** (algoritmai.cpp)
   - Derivative-free optimization
   - Works with simplex method

## Project Structure

- **lib.hpp** - Common utilities:
  - Objective function and gradient computation
  - Optimization result structure
  - Helper functions (validation, distance, etc.)

- **algoritmai.cpp** - Algorithm implementations:
  - `gradientDescent()`
  - `steepestDescent()`
  - `nelderMead()`

- **main.cpp** - Main workflow:
  - Calculates function values at starting points (X0, X1, Xm)
  - Runs all three algorithms from each starting point
  - Displays results in terminal
  - Prompts for SFML visualization

## Configuration

Before compiling, set your student ID digits in main.cpp:
```cpp
int a = 0;  // TODO: Set your student ID digit
int b = 0;  // TODO: Set your student ID digit
```

The project uses these values to compute starting point Xm = (a/10, b/10).

## Compilation

### Using CMake (Recommended)

Requires SFML 2.5+ installed:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Manual Compilation

With g++:
```bash
g++ -std=c++17 main.cpp algoritmai.cpp -o optimization_methods -O2 -lsfml-graphics -lsfml-window -lsfml-system
```

## Running

```bash
./optimization_methods
```

The program will:
1. Display function and gradient values at starting points
2. Run all three algorithms from each starting point
3. Show results (solution, minimum value, steps, evaluations)
4. Ask if you want to visualize the results with SFML

### Visualization

When prompted, enter 'y' to open an SFML window showing:
- Contour plot of the objective function
- Path taken by each algorithm (different colors):
  - **Red** - Gradient Descent
  - **Green** - Steepest Descent
  - **Blue** - Nelder-Mead
- Final solutions marked as larger circles

## Output Example

```
=== Optimization Methods Laboratory ===
Student ID digits: a = 5, b = 7

Starting points:
  X0 = (0, 0)
  X1 = (1, 1)
  Xm = (0.5, 0.7)

=== Function and Gradient Values at Starting Points ===
X0 = (0, 0):
  f(x,y) = 0
  Gradient = (0, 0)
  Gradient magnitude = 0
  
...

=== Running Optimization Algorithms ===
Starting from (0, 0):
  Gradient Descent:
    Solution: (0.33333, 0.33333)
    Min value: -0.01157
    Steps: 342, Evaluations: 343
...

Would you like to visualize the objective function and search paths? (y/n): 
```

## Notes

- The algorithms stop when gradient magnitude < 1e-6 or max iterations (10000) is reached
- For starting points with gradient = 0 (like X0), algorithms may not move
- X1 = (1, 1) is outside the valid region, so algorithms may terminate early
- Results are displayed with 8 decimal places precision
