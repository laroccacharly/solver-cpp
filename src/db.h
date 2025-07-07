#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "sqlite_orm/sqlite_orm.h"
#include <chrono>
#include <optional>

using namespace std;
using namespace sqlite_orm;

// Helper function
inline int64_t unix_now() {
    auto now = chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto value = chrono::duration_cast<chrono::seconds>(epoch);
    return value.count();
}

// Types 
struct Instance {
    string id; 
    string name;
    int64_t created_at = unix_now();
    bool selected = false;
    int num_bin_variables;
    int num_int_variables; 
    float best_known_obj_val = 1e10;
};

// https://docs.gurobi.com/projects/optimizer/en/current/concepts/attributes/types.html#secattributetypes
struct GRBAttributes {
    int id = -1; 
    int job_id = -1;
    double MIPGap; 
    double PrimalGap = -1.0;
    double Runtime; 
    int SolCount;
    double NodeCount; 
    int Status; 
    double ObjVal; 
    double MaxMemUsed; // GB
    int NumVars;
    int NumConstrs;
    int NumBinVars;
    int NumIntVars;
    string solution;
}; 

struct CallbackMetric {
    int id = -1; 
    int job_id = -1;
    int non_zero_count;
    int phase;
    int solcnt;
    int64_t elapsed_ms;
};

struct Job { 
    int id; 
    string instance_id; 
    int time_limit_s = 10;
    string group_name = ""; 
    bool enable_callback = false;
    bool warm_start = false;
    bool enable_lns = false;
    int seed = 0;
    float fixing_ratio = 0.20; // 20% of the variables are fixed
    int64_t created_at = unix_now();
}; 


void sync_db();
void seed_instances();
vector<string> get_instance_names();
vector<Instance> get_instances();
vector<Instance> get_selected_instances();
void batch_insert_metrics(vector<CallbackMetric>& metrics, int batch_size = 1000);
optional<vector<int>> get_best_solution_for_instance_from_db(string instance_id);

inline auto get_storage() {
    return make_storage("data/db.sqlite",
        make_table("instances", 
            make_column("id", &Instance::id, primary_key()), 
            make_column("name", &Instance::name),
            make_column("created_at", &Instance::created_at),
            make_column("selected", &Instance::selected),
            make_column("num_bin_variables", &Instance::num_bin_variables),
            make_column("num_int_variables", &Instance::num_int_variables),
            make_column("best_known_obj_val", &Instance::best_known_obj_val)
        ),  
        make_table("jobs", 
            make_column("id", &Job::id, primary_key().autoincrement()),
            make_column("instance_id", &Job::instance_id),
            make_column("group_name", &Job::group_name),
            make_column("time_limit_s", &Job::time_limit_s),
            make_column("enable_callback", &Job::enable_callback),
            make_column("warm_start", &Job::warm_start),
            make_column("enable_lns", &Job::enable_lns),
            make_column("fixing_ratio", &Job::fixing_ratio),
            make_column("seed", &Job::seed),
            make_column("created_at", &Job::created_at)
        ),
        make_table("grb_attributes",
            make_column("id", &GRBAttributes::id, primary_key().autoincrement()),
            make_column("job_id", &GRBAttributes::job_id),
            make_column("mip_gap", &GRBAttributes::MIPGap),
            make_column("primal_gap", &GRBAttributes::PrimalGap),
            make_column("runtime", &GRBAttributes::Runtime),
            make_column("sol_count", &GRBAttributes::SolCount),
            make_column("node_count", &GRBAttributes::NodeCount),
            make_column("status", &GRBAttributes::Status),
            make_column("obj_val", &GRBAttributes::ObjVal),
            make_column("max_mem_used", &GRBAttributes::MaxMemUsed),
            make_column("solution", &GRBAttributes::solution)
        ),
        make_table("callback_metrics",
            make_column("id", &CallbackMetric::id, primary_key().autoincrement()),
            make_column("non_zero_count", &CallbackMetric::non_zero_count),
            make_column("phase", &CallbackMetric::phase),
            make_column("solcnt", &CallbackMetric::solcnt),
            make_column("elapsed_ms", &CallbackMetric::elapsed_ms),
            make_column("job_id", &CallbackMetric::job_id)
        )
    );
}

