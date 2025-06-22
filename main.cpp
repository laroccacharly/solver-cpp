#include <fmt/core.h>
#include "src/diet_c++.h"

using namespace std;

int main() {
    fmt::print("Starting Gurobi example...\n");
    auto result = solveDiet();
    fmt::print("Result: {}\n", result);
    return 0;
} 