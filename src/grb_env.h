#pragma once
#include "gurobi_c++.h"

class GurobiEnvironment {
    public:
        static GRBEnv& getEnv() {
            static GRBEnv env;
            static bool initialized = false;
            if (!initialized) {
                // Configure global Gurobi settings here
                env.set(GRB_IntParam_OutputFlag, 1);  // Enable output
                initialized = true;
            }
            return env;
        }
    };