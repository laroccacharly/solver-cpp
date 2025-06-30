#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "sqlite_orm/sqlite_orm.h"
#include <chrono>

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
};

// https://docs.gurobi.com/projects/optimizer/en/current/concepts/attributes/types.html#secattributetypes
struct GRBAttributes {
    int id = -1; 
    int job_id = -1;
    double MIPGap; 
    double Runtime; 
    int SolCount;
    double NodeCount; 
    int Status; 
    double ObjVal; 
    double MaxMemUsed; // GB
}; 

struct CallbackMetric {
    int id = -1; 
    int job_id = -1;
    int non_zero_count;
    int phase;
    int solcnt;
    int64_t elapsed_ms;
};

struct Strategy {
    string id = "default"; 
    int64_t created_at;
}; 

struct Job { 
    int id; 
    string instance_id; 
    int time_limit_s = 10;
    string strategy_id = "default"; 
    int64_t created_at = unix_now();
}; 


void sync_db();
void seed_instances();
vector<string> get_instance_names();
vector<Instance> get_instances();
void batch_insert_metrics(vector<CallbackMetric>& metrics);

inline auto get_storage() {
    return make_storage("data/db.sqlite",
        make_table("instances", 
            make_column("id", &Instance::id, primary_key()), 
            make_column("name", &Instance::name),
            make_column("created_at", &Instance::created_at)
        ),
        make_table("strategies", 
            make_column("id", &Strategy::id, primary_key()),
            make_column("created_at", &Strategy::created_at)
        ),
        make_table("jobs", 
            make_column("id", &Job::id, primary_key().autoincrement()),
            make_column("instance_id", &Job::instance_id),
            make_column("strategy_id", &Job::strategy_id),
            make_column("time_limit_s", &Job::time_limit_s),
            make_column("created_at", &Job::created_at)
        ),
        make_table("grb_attributes",
            make_column("id", &GRBAttributes::id, primary_key().autoincrement()),
            make_column("job_id", &GRBAttributes::job_id),
            make_column("mip_gap", &GRBAttributes::MIPGap),
            make_column("runtime", &GRBAttributes::Runtime),
            make_column("sol_count", &GRBAttributes::SolCount),
            make_column("node_count", &GRBAttributes::NodeCount),
            make_column("status", &GRBAttributes::Status),
            make_column("obj_val", &GRBAttributes::ObjVal),
            make_column("max_mem_used", &GRBAttributes::MaxMemUsed)
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

