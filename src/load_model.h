#pragma once
#include "grb_env.h"
#include "gurobi_c++.h"
#include "fmt/core.h"

using namespace std;

inline string getMpsDir() {
    const char* mps_files_dir = getenv("MPS_FILES_DIR");
    if (mps_files_dir == nullptr) {
        fmt::print("Error: The environment variable MPS_FILES_DIR is not set.\n");
        return "";
    }
    return string(mps_files_dir);
}

inline GRBModel loadModel(const string& instance_name) {
    string path = fmt::format("{}/{}.mps", getMpsDir(), instance_name); 
    return GRBModel(GurobiEnvironment::getEnv(), path);
}
