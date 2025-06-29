#include <fmt/core.h>
// #include "src/diet_c++.h"
#include "src/solve_mps.h"
#include "src/db.h"

using namespace std;

int main() {
    fmt::print("Starting main...\n");
    sync_db();
    seed_instances();
    solveOneMps();
    return 0;
} 