// Solves an MPS file using Gurobi. 

#include "gurobi_c++.h"
#include "fmt/core.h"
#include <string>
#include <chrono>
#include <fstream>

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
  private:
    GRBVar* binary_vars;
    int num_binary_vars;
    vector<CallbackMetric> metrics;
    chrono::steady_clock::time_point start_time;

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



  public:
    CallbackState(GRBVar* vars, int count) {
        binary_vars = vars;
        num_binary_vars = count;
        start_time = chrono::steady_clock::now();
    }

    ~CallbackState() {
      // We don't delete binary_vars here since it's managed by the caller
    }
    void printMetrics() {
      fmt::print("Metrics:\n");
      for (const auto& metric : metrics) {
        fmt::print("Non-zero count: {}\n", metric.non_zero_count);
        fmt::print("Phase: {}\n", metric.phase);
        fmt::print("Solution count: {}\n", metric.solcnt);
        fmt::print("Elapsed time (ms): {}\n", metric.elapsed_ms);
      }
    }

    void saveMetricsCSV() {
        string filename = "metrics.csv";
        ofstream file(filename);
        file << "non_zero_count,phase,solcnt,elapsed_ms\n";
        for (const auto& metric : metrics) {
            file << metric.non_zero_count << "," 
                << metric.phase << "," 
                << metric.solcnt << "," 
                << metric.elapsed_ms 
                << endl;
        }
        file.close();
    }
  protected:
    void callback () {
      try {
        if (where == GRB_CB_MIPNODE) {
          handleMipNode();
        }
      } catch (GRBException e) {
        fmt::print("Error: {}\n", e.getMessage());
      }
    }
};

void solveMpsMain() {
    std::string path = "data/academictimetablesmall.mps";
    // std::string path = "data/50v-10.mps";
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env, path);

    model.set(GRB_DoubleParam_TimeLimit, 10);
    // model.set(GRB_IntParam_SolutionLimit, 20);
    int num_variables = model.get(GRB_IntAttr_NumVars);
    
    vector<int> binary_indices;
    for (int i = 0; i < num_variables; i++) {
        GRBVar var = model.getVar(i);
        if (isBinary(var)) {
            binary_indices.push_back(i);
        }
    }

    GRBVar* binary_variables = new GRBVar[binary_indices.size()];
    for (int i = 0; i < binary_indices.size(); i++) {
        binary_variables[i] = model.getVar(binary_indices[i]);
    }

    fmt::print("Model has {} binary variables\n", binary_indices.size());
    fmt::print("Model has {} variables\n", num_variables);

    CallbackState callbackState(binary_variables, binary_indices.size());
    model.setCallback(&callbackState);

    model.optimize();

    fmt::print("Objective: {}\n", model.get(GRB_DoubleAttr_ObjVal));
    // callbackState.printMetrics();
    callbackState.saveMetricsCSV();
    // Clean up
    delete[] binary_variables;
}

void solveMps() {
    try {
        solveMpsMain();
    } catch (GRBException e) {
        fmt::print("Error: {}\n", e.getMessage());
    }
}

