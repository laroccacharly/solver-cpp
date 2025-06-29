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
        Instance instance = {name, name};
        storage.replace(instance);
    }
    vector<Instance> instances = get_instances();
    fmt::print("Seeded {} instances\n", instances.size());
}





