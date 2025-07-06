#pragma once

#include "gurobi_c++.h"

using namespace std;

inline bool isBinary(GRBVar& var) {
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

inline vector<GRBVar> getBinaryVariables(GRBModel& model) {
    vector<GRBVar> binary_variables;
    for (int i = 0; i < model.get(GRB_IntAttr_NumVars); i++) {
        GRBVar var = model.getVar(i);
        if (isBinary(var)) {
            binary_variables.push_back(var);
        }
    }
    return binary_variables;
}