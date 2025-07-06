#include "db.h"
#include <fstream>
#include "fmt/core.h"
#include "sqlite_orm/sqlite_orm.h"
#include <algorithm>
#include <sstream>
#include "load_model.h"
#include "binary_variables.h"

using namespace std;
using namespace sqlite_orm;

vector<string> get_instance_names() {
    ifstream file("data/instance_names.txt");
    vector<string> instance_names;
    string line;
    while (getline(file, line)) {
        instance_names.push_back(line);
    }
    fmt::print("Loaded {} instance names\n", instance_names.size());
    return instance_names;
}

void sync_db() {
    auto storage = get_storage();
    fmt::print("Syncing db schema\n");
    storage.sync_schema(true);
}

vector<Instance> get_instances() {
    auto storage = get_storage();
    return storage.get_all<Instance>();
}

void seed_instances() {
    vector<string> instance_names = get_instance_names();
    auto storage = get_storage();
    for (string name : instance_names) {
        GRBModel model = loadModel(name);
        vector<GRBVar> binary_variables = getBinaryVariables(model);
        int num_bin_variables = binary_variables.size();
        if (num_bin_variables == 0) {
            fmt::print("Instance {} has no binary variables, skipping\n", name);
            continue;
        }
        Instance instance = {
            .id = name, 
            .name = name, 
            .num_bin_variables = num_bin_variables, 
            .num_int_variables = model.get(GRB_IntAttr_NumVars)
        };
        storage.replace(instance);
    }
    vector<Instance> instances = get_instances();
    fmt::print("Seeded {} instances\n", instances.size());
}

void batch_insert_metrics(vector<CallbackMetric>& metrics, int batch_size) {
    auto storage = get_storage();
    for (size_t i = 0; i < metrics.size(); i += batch_size) {
        auto batch_start = metrics.begin() + i;
        auto batch_end = metrics.begin() + std::min(i + batch_size, metrics.size());
        storage.insert_range(batch_start, batch_end);
    }
}

optional<vector<int>> get_best_solution_for_instance(string instance_id) {
    auto storage = get_storage();
    
    auto results = storage.select(
        object<GRBAttributes>(),
        join<Job>(on(c(&GRBAttributes::job_id) == &Job::id)),
        where(c(&Job::instance_id) == instance_id and c(&GRBAttributes::solution) != ""),
        order_by(&GRBAttributes::ObjVal).asc(),
        limit(1)
    );

    if (results.empty()) {
        return std::nullopt;
    }

    auto& result = results[0];
    string& solution_str = result.solution;
    if (solution_str.empty()) {
        return std::nullopt;
    }
    fmt::print("Found solution with obj_val: {} for instance: {}\n", (double)result.ObjVal, instance_id);
    stringstream ss(solution_str);
    string item;
    vector<int> solution;
    while (getline(ss, item, ',')) {
        if (!item.empty()) {
            solution.push_back(stoi(item));
        }
    }
    return solution;
}




