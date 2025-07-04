#include <vector>
#include <string>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include "gurobi_c++.h"

using namespace std;
using namespace fmt;

inline string getMpsDir() {
    const char* mps_files_dir = getenv("MPS_FILES_DIR");
    if (mps_files_dir == nullptr) {
        fmt::print("Error: The environment variable MPS_FILES_DIR is not set.\n");
        return "";
    }
    return string(mps_files_dir);
}

inline string convertVectorToString(vector<int>& solution) {
    return format("{}", join(solution, ","));
}

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

// A solution is a vector of indices for the non zero variables 
inline vector<int> get_best_solution_from_model(GRBModel& model, vector<GRBVar>& binary_variables, double tolerance = 0.001) {
    vector<int> solution;
    for (int i = 0; i < binary_variables.size(); i++) {
      GRBVar var = binary_variables[i];
      double value = var.get(GRB_DoubleAttr_X);
      if (value > tolerance) {
        solution.push_back(i);
      }
    } 
    return solution;
  }
