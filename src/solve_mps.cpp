// Solves an MPS file using Gurobi. 

#include "gurobi_c++.h"
#include "fmt/core.h"

using namespace std;

void solveMpsMain() {
    std::string path = "data/academictimetablesmall.mps";
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env, path);

    model.set(GRB_DoubleParam_TimeLimit, 10);
    model.optimize();

    fmt::print("Objective: {}\n", model.get(GRB_DoubleAttr_ObjVal));
}

void solveMps() {
    try {
        solveMpsMain();
    } catch (GRBException e) {
        fmt::print("Error: {}\n", e.getMessage());
    }
}

