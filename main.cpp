#include <fmt/core.h>
#include "src/db.h"
#include "src/solve_mps.h"

using namespace std;

int main() {
    fmt::print("Starting main...\n");
    sync_db();
    seed_instances();
    solveAll();
    return 0;
} 