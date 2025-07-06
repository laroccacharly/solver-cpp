#include <vector>
#include <string>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include "gurobi_c++.h"
#include <random>
#include <algorithm>

using namespace std;
using namespace fmt;


inline string convertVectorToString(vector<int>& solution) {
    return format("{}", join(solution, ","));
}

// Sample without replacement from the set {0, 1, ..., n-1}
// Returns a vector of sample_size integers sampled uniformly without replacement
inline vector<int> sample_without_replacement(int n, int sample_size, mt19937& rng) {

    // Create vector with elements {0, 1, ..., n-1}
    vector<int> population;
    population.reserve(n);
    for (int i = 0; i < n; i++) {
        population.push_back(i);
    }
    
    // Shuffle the entire vector
    shuffle(population.begin(), population.end(), rng);
    
    // Take the first sample_size elements
    population.resize(sample_size);
    return population;
}

inline vector<int> sample_percentage(int n, double percentage, mt19937& rng) {
    int sample_size = static_cast<int>(n * percentage);
    return sample_without_replacement(n, sample_size, rng);
}

inline vector<int> sample_percentage(int n, double percentage) {
    random_device rd;
    mt19937 rng(rd());
    return sample_percentage(n, percentage, rng);
}

inline vector<int> sample_percentage(int n, double percentage, int seed) {
    mt19937 rng(seed);
    return sample_percentage(n, percentage, rng);
}

// A solution is a vector of indices for the non zero variables 
inline vector<int> get_best_solution_from_model(GRBModel& model, vector<GRBVar>& binary_variables, double tolerance = 0.001) {
    vector<int> solution;
    for (int i = 0; i < binary_variables.size(); i++) {
      GRBVar var = binary_variables[i];
      double value = var.get(GRB_DoubleAttr_X);
      if (value > tolerance) {
        solution.push_back(i);
      }
    } 
    return solution;
  }
