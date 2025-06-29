#include "db.h"
#include <fstream>
#include "fmt/core.h"
#include "sqlite_orm/sqlite_orm.h"

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

struct Instance {
    int id;
    string name;
};

auto get_storage() {
    return make_storage("data/db.sqlite", 
        make_table("instances", 
        make_column("id", &Instance::id, primary_key().autoincrement()), 
        make_column("name", &Instance::name)
    ));
}

void sync_db() {
    auto storage = get_storage();
    storage.sync_schema();

}

vector<Instance> get_instances() {
    auto storage = get_storage();
    return storage.get_all<Instance>();
}

void seed_instances() {
    vector<string> instance_names = get_instance_names();
    auto storage = get_storage();
    for (string name : instance_names) {
        Instance instance = {-1, name};
        storage.insert(instance);
    }
    vector<Instance> instances = get_instances();
    fmt::print("Seeded {} instances\n", instances.size());

}





