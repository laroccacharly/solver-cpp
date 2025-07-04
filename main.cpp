#include <cxxopts.hpp>
#include <fmt/core.h>
#include <functional>
#include <map>
#include <fmt/ranges.h>
#include "src/db.h"
#include "src/solve_mps.h"

using namespace std;

struct Action {
    string name;
    function<int()> func;
};

vector<Action> get_actions() {
    return {
        Action{"syncdb", []() { sync_db(); return 0; }},
        Action{"solveall", []() { solveAll(); return 0; }},
    };
}

int main(int argc, char **argv) {
    vector<Action> actions = get_actions();
    map<string, function<int()>> action_map;
    vector<string> action_names;
    for (const auto& action : actions) {
        action_names.push_back(action.name);
        action_map[action.name] = action.func;
    }
    string action_names_string = fmt::format("{}", fmt::join(action_names, ", "));

    cxxopts::Options options("Solver", "Example project to solve MPS files with Gurobi and LNS");
    options.add_options()("h,help", "Print usage")(
        "a,action", action_names_string,
        cxxopts::value<string>());

    auto result = options.parse(argc, argv);

    if (result.count("help") || result.arguments().size() == 0) {
    fmt::print("{}\n", options.help());
    return 0;
    }

    if (result.count("action")) {
        auto action = result["action"].as<string>();
        if (action_map.find(action) != action_map.end()) {
            return action_map[action]();
        } else {
            fmt::print(stderr, "Unknown action: {}\n", action);
            fmt::print(stderr, "Available actions: {}\n", action_names_string);
            return 1;
        }
    }

    return 0;
} 