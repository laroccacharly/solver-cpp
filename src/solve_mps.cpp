// Solves an MPS file using Gurobi. 

#include "gurobi_c++.h"
#include "fmt/core.h"
#include <string>

using namespace std;

bool isBinary(GRBVar& var) {
    char vtype = var.get(GRB_CharAttr_VType);
    
    if (vtype == 'B') {
        return true;
    }
    
    if (vtype == 'I') {
        double lb = var.get(GRB_DoubleAttr_LB);
        double ub = var.get(GRB_DoubleAttr_UB);
        if (lb == 0.0 && ub == 1.0) {
            return true;
        }
    }
    
    return false;
}

void solveMpsMain() {
    std::string path = "data/academictimetablesmall.mps";
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env, path);

    model.set(GRB_DoubleParam_TimeLimit, 10);

    int num_variables = model.get(GRB_IntAttr_NumVars);
    int num_binary_variables = 0;
    for (int i = 0; i < num_variables; i++) {
        GRBVar var = model.getVar(i);
        if (isBinary(var)) {
            num_binary_variables++;
        }
    }
    fmt::print("Model has {} binary variables\n", num_binary_variables);
    fmt::print("Model has {} variables\n", num_variables);

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

