// Solves an MPS file using Gurobi. 

#include "solve_mps.h"
#include "db.h" 

#include "gurobi_c++.h"
#include "fmt/core.h"
#include <string>
#include <chrono>
#include <fstream>
#include <cstdlib>

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

struct CallbackMetric {
    int non_zero_count;
    int phase;
    int solcnt;
    int64_t elapsed_ms;
};

class CallbackState: public GRBCallback
{
  public:
    CallbackState(GRBVar* binary_vars, int num_binary_vars, string instance_name) {
        this->instance_name = instance_name;
        this->binary_vars = binary_vars;
        this->num_binary_vars = num_binary_vars;
        start_time = chrono::steady_clock::now();
    }

    void printSummary() {
      fmt::print("Summary:\n");
      // Print the number of solutions found
      fmt::print("Number of solutions found using callback: {}\n", metrics.size());
    }

    void saveMetricsCSV() {
        string filename = fmt::format("data/{}_metrics.csv", instance_name);
        ofstream file(filename);
        file << "non_zero_count,phase,solcnt,elapsed_ms,instance_name\n";
        for (const auto& metric : metrics) {
            file << metric.non_zero_count << "," 
                << metric.phase << "," 
                << metric.solcnt << "," 
                << metric.elapsed_ms << ","
                << instance_name
                << endl;
        }
        file.close();
    }

  private:
    GRBVar* binary_vars;
    int num_binary_vars;
    vector<CallbackMetric> metrics;
    chrono::steady_clock::time_point start_time;
    int error_count = 0;
    string instance_name;
    
    void handleMipNode() {
      int solcnt = getIntInfo(GRB_CB_MIPNODE_SOLCNT);
      int phase = getIntInfo(GRB_CB_MIPNODE_PHASE);
      double* x = getNodeRel(binary_vars, num_binary_vars);

      vector<int> non_zero_indices;
      for (int i = 0; i < num_binary_vars; i++) {
        double value = x[i];
        if (value > 0.0) {
            non_zero_indices.push_back(i);
        }
      }
      auto now = chrono::steady_clock::now();
      auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(now - start_time).count();
      metrics.push_back({
        (int)non_zero_indices.size(), 
        phase, 
        solcnt, 
        elapsed_ms,
      }); 
      delete[] x;
    }

  protected:
    void callback () {
      try {
        if (where == GRB_CB_MIPNODE) {
          handleMipNode();
        }
      } catch (GRBException e) {
        error_count++;
      }
    }
};

string getMpsDir() {
    const char* mps_files_dir = getenv("MPS_FILES_DIR");
    if (mps_files_dir == nullptr) {
        fmt::print("Error: The environment variable MPS_FILES_DIR is not set.\n");
        return "";
    }
    return string(mps_files_dir);
}

struct SolveConfig {
  string instance_name;
  int time_limit_s = 10;
};

void solveMpsMain(SolveConfig config) {
    string instance_name = config.instance_name;

    string path = fmt::format("{}/{}.mps", getMpsDir(), instance_name);
    fmt::print("Solving instance: {}\n", path);
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env, path);

    model.set(GRB_DoubleParam_TimeLimit, config.time_limit_s);
    // model.set(GRB_IntParam_SolutionLimit, 20);
    int num_variables = model.get(GRB_IntAttr_NumVars);
    
    vector<GRBVar> binary_variables;
    for (int i = 0; i < num_variables; i++) {
        GRBVar var = model.getVar(i);
        if (isBinary(var)) {
            binary_variables.push_back(var);
        }
    }

    fmt::print("Model has {} binary variables\n", binary_variables.size());
    fmt::print("Model has {} variables\n", num_variables);

    CallbackState callbackState(
      binary_variables.data(), 
      binary_variables.size(),
      instance_name
    );
    model.setCallback(&callbackState);

    model.optimize();

    fmt::print("Objective: {}\n", model.get(GRB_DoubleAttr_ObjVal));
    callbackState.printSummary();
    callbackState.saveMetricsCSV();
    // Clean up is now automatic thanks to std::vector
}

void solveMps(SolveConfig config) {
    try {
        solveMpsMain(config);
    } catch (GRBException e) {
        fmt::print("Error: {}\n", e.getMessage());
    }
}

void solveOneMps() {
  vector<string> instance_names = get_instance_names();
  string instance_name = instance_names[0];
  SolveConfig config = {instance_name, 10};
  solveMps(config);
}

void solveAllMps() {
  vector<string> instance_names = get_instance_names();
  fmt::print("Solving {} instances\n", instance_names.size());
  for (const auto& instance_name : instance_names) {
    SolveConfig config = {instance_name, 10};
    solveMps(config);
  }
}