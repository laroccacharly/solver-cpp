#include <fmt/core.h>
#include "gurobi_c++.h"

using namespace std;

int main() {
    fmt::print("Starting Gurobi example...\n");
    
    try {
        // Create an environment
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "solver.log");
        env.start();

        // Create an empty model
        GRBModel model = GRBModel(env);

        // Create variables
        GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
        GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
        GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

        // Set objective: maximize x + y + 2 z
        model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);

        // Add constraint: x + 2 y + 3 z <= 4
        model.addConstr(x + 2 * y + 3 * z <= 4, "c0");

        // Add constraint: x + y >= 1
        model.addConstr(x + y >= 1, "c1");

        // Optimize model
        model.optimize();

        fmt::print("{} = {:.2f}\n", x.get(GRB_StringAttr_VarName), x.get(GRB_DoubleAttr_X));
        fmt::print("{} = {:.2f}\n", y.get(GRB_StringAttr_VarName), y.get(GRB_DoubleAttr_X));
        fmt::print("{} = {:.2f}\n", z.get(GRB_StringAttr_VarName), z.get(GRB_DoubleAttr_X));
        fmt::print("Obj: {:.2f}\n", model.get(GRB_DoubleAttr_ObjVal));

    } catch(GRBException e) {
        fmt::print("Error code = {}\n", e.getErrorCode());
        fmt::print("{}\n", e.getMessage());
        return 1;
    } catch(...) {
        fmt::print("Exception during optimization\n");
        return 1;
    }

    return 0;
} 