#include "solve_mps.h"
#include "db.h" 
#include "utils.h"
#include "load_model.h"
#include "binary_variables.h"

#include "gurobi_c++.h"
#include "fmt/core.h"
#include <string>
#include <chrono>
#include <fstream>
#include <cstdlib>

using namespace std;

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

    vector<CallbackMetric>& getMetrics() {
        return metrics;
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
      int non_zero_count = non_zero_indices.size();
      metrics.push_back({
        .non_zero_count = non_zero_count, 
        .phase = phase, 
        .solcnt = solcnt, 
        .elapsed_ms = elapsed_ms,
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


GRBAttributes createGRBAttributes(GRBModel& model) {
  return GRBAttributes{
    .MIPGap = model.get(GRB_DoubleAttr_MIPGap), 
    .Runtime = model.get(GRB_DoubleAttr_Runtime),
    .SolCount = model.get(GRB_IntAttr_SolCount),
    .NodeCount = model.get(GRB_DoubleAttr_NodeCount), 
    .Status = model.get(GRB_IntAttr_Status),
    .ObjVal = model.get(GRB_DoubleAttr_ObjVal),
    .MaxMemUsed = model.get(GRB_DoubleAttr_MaxMemUsed),
    .NumVars = model.get(GRB_IntAttr_NumVars),
    .NumConstrs = model.get(GRB_IntAttr_NumConstrs),
    .NumBinVars = model.get(GRB_IntAttr_NumBinVars),
    .NumIntVars = model.get(GRB_IntAttr_NumIntVars),
  };
}

static void applyWarmStart(const string& instance_name, vector<GRBVar>& binary_variables) {
    auto best_solution = get_best_solution_for_instance(instance_name);
    if (!best_solution) {
        fmt::print("No best solution found for instance, skipping warm start\n");
        return;
    }

    // start by setting all binary variables to 0
    for (int i = 0; i < binary_variables.size(); i++) {
        binary_variables[i].set(GRB_DoubleAttr_Start, 0.0);
    }

    fmt::print("Found best solution for instance, applying warm start\n");
    vector<int> solution = *best_solution;
    for (int var_index : solution) {
        binary_variables[var_index].set(GRB_DoubleAttr_Start, 1.0);
    }
}

void applyLNS(GRBModel& model, Job& job) {
    string instance_name = job.instance_id;
    auto best_solution = get_best_solution_for_instance(instance_name);
    if (!best_solution) {
        fmt::print("No best solution found for instance, skipping LNS\n");
        return;
    }
    // We use random LNS where we sample without replacement binary_variables (fixing_ratio * num_binary_variables) 
    vector<int> one_indices = *best_solution;
    vector<GRBVar> binary_variables = getBinaryVariables(model);
    int num_binary_variables = binary_variables.size();
    vector<int> fixing_indices = sample_percentage(num_binary_variables, job.fixing_ratio, job.seed);

    // build solution 
    vector<float> solution; 
    for (int i = 0; i < num_binary_variables; i++) {
        solution.push_back(0.0);
    }
    for (int var_index : one_indices) {
        solution[var_index] = 1.0;
    }

    // add LNS constraint 
    GRBLinExpr lns_expression = 0;
    for (int var_index : fixing_indices) {
        GRBVar var = binary_variables[var_index];
        double value = solution[var_index];
        lns_expression += var * (1 - value); // if value is 0, we must fix var to 0
        lns_expression += (1 - var) * value; // if value is 1, we must fix var to 1
    }
    model.addConstr(lns_expression == 0, "LNS");
    fmt::print("Added LNS constraint with {} variables fixed\n", fixing_indices.size());
}

void _solveJob(Job job) {
    string instance_name = job.instance_id;
    GRBModel model = loadModel(instance_name);
    model.set(GRB_DoubleParam_TimeLimit, job.time_limit_s);
    model.set(GRB_IntParam_Seed, job.seed);
    
    vector<GRBVar> binary_variables = getBinaryVariables(model);

    if (job.warm_start) {
        applyWarmStart(instance_name, binary_variables);
    }

    if (job.enable_lns) {
        applyLNS(model, job);
    }

    CallbackState callbackState(
      binary_variables.data(), 
      binary_variables.size(),
      instance_name
    );
    model.setCallback(&callbackState);

    model.optimize();

    fmt::print("Objective: {}\n", model.get(GRB_DoubleAttr_ObjVal));
    callbackState.printSummary();

    auto storage = get_storage();
    int job_id = storage.insert(job);
    job.id = job_id;
    fmt::print("Inserted job with id: {}\n", job.id);

    GRBAttributes attributes = createGRBAttributes(model);
    attributes.job_id = job.id;
    if (model.get(GRB_IntAttr_SolCount) > 0) {
      vector<int> solution = get_best_solution_from_model(model, binary_variables);
      string solution_str = convertVectorToString(solution);
      fmt::print("Found solution for instance: {}\n", instance_name);
      attributes.solution = solution_str;
    }

    storage.insert(attributes);

    auto& metrics = callbackState.getMetrics();
    for (auto& metric : metrics) {
        metric.job_id = job.id;
    }
    batch_insert_metrics(metrics);
}

void solveJob(Job job) {
    try {
        _solveJob(job);
    } catch (GRBException e) {
        fmt::print("Error: {}\n", e.getMessage());
    }
}

void solveGRBOnly() { 
  vector<Instance> instances = get_instances();
  int instance_count = instances.size();
  for (int i = 0; i < instance_count; i++) {
    fmt::print("Solving instance {}/{}\n", i + 1, instance_count);
    Instance instance = instances[i];
    Job job = {
      .instance_id = instance.id,
      .time_limit_s = 10,
      .group_name = "grb_only",
    };
    solveJob(job);
  }
}

void solveWarmStart() {
  fmt::print("Running job group: warm_start\n");
  vector<Instance> instances = get_selected_instances();
  for (Instance& instance : instances) {
    Job job = {
      .instance_id = instance.id,
      .time_limit_s = 10,
      .warm_start = true,
      .group_name = "warm_start",
    };
    solveJob(job);
  }
}

void solveLNS() {
  fmt::print("Running job group: lns\n");
  vector<Instance> instances = get_selected_instances();
  vector<Job> jobs;
  vector<float> fixing_ratios = {0.2, 0.5, 0.8};
  vector<int> seeds = {0, 1, 2};
  string group_name; 
  for (float fixing_ratio : fixing_ratios) {
    group_name = fmt::format("lns_{:.2f}", fixing_ratio);
    for (int seed : seeds) {
      for (Instance& instance : instances) {
        Job job = {
          .instance_id = instance.id,
          .time_limit_s = 10,
          .warm_start = true,
          .enable_lns = true,
          .fixing_ratio = fixing_ratio,
          .seed = seed,
          .group_name = group_name,
        };
        jobs.push_back(job);
      }
    }
  }
  fmt::print("Solving {} jobs\n", jobs.size());
  for (Job& job : jobs) {
    solveJob(job);
  }
}

void solveSelectedInstances() {
  vector<Instance> instances = get_instances();
  vector<Instance> selected_instances;
  for (Instance& instance : instances) {
    if (instance.selected) {
      selected_instances.push_back(instance);
    }
  }
  fmt::print("Solving {} selected instances\n", selected_instances.size());
  for (Instance& instance : selected_instances) {
    Job job = {
      .instance_id = instance.id,
      .time_limit_s = 10,
      .warm_start = true,
      .enable_lns = true,
      .fixing_ratio = 0.20,
    };
    solveJob(job);
  }
}