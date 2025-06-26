/* Copyright 2025, Gurobi Optimization, LLC */

/* Solve the classic diet model, showing how to add constraints
   to an existing model. */

#include "diet_c++.h"
#include "gurobi_c++.h"
#include "fmt/core.h"
using namespace std;

void printSolution(GRBModel& model, int nCategories, int nFoods,
                   GRBVar* buy, GRBVar* nutrition);


struct NutrientValue { 
  string name;
  double value;
};

struct Nutrient {
  string name;
  double min;
  double max;
  GRBVar var;  

  void show() { 
    fmt::print("Nutrient: {}\n", name);
    fmt::print("Recommended amount: {}\n", var.get(GRB_DoubleAttr_X));
  }
};

vector<Nutrient> getNutrients() {
  return {
    {"calories", 1800, 2200},
    {"protein", 91, GRB_INFINITY},
    {"fat", 0, 65},
    {"sodium", 0, 1779}
  };
}

// a function that returns a vector of nutrients
struct Food {
  string name;
  double cost;
  vector<NutrientValue> nutrients;
  GRBVar var;
  double getNutrientValueFromName(string nutrient_name) {
    double value = 0;
    for (auto nutrient : nutrients) {
      if (nutrient.name == nutrient_name) {
        value = nutrient.value;
      }
    }
    return value;
  } 

  void show() { 
    fmt::print("Food: {}\n", name);
    double amount = var.get(GRB_DoubleAttr_X);
    fmt::print("Recommended amount: {}\n", amount);
  }
};

vector<Food> getFoods() {
  return {
    {"hamburger", 2.49, {{"calories", 410}, {"protein", 24}, {"fat", 26}, {"sodium", 730}}},
    {"chicken", 2.89, {{"calories", 420}, {"protein", 32}, {"fat", 10}, {"sodium", 1190}}},
    {"hot dog", 1.50, {{"calories", 560}, {"protein", 20}, {"fat", 32}, {"sodium", 1800}}},
    {"fries", 1.89, {{"calories", 380}, {"protein", 4}, {"fat", 19}, {"sodium", 270}}},
    {"macaroni", 2.09, {{"calories", 320}, {"protein", 12}, {"fat", 10}, {"sodium", 930}}},
    {"pizza", 1.99, {{"calories", 320}, {"protein", 15}, {"fat", 12}, {"sodium", 820}}},
    {"salad", 2.49, {{"calories", 320}, {"protein", 31}, {"fat", 12}, {"sodium", 1230}}},
    {"milk", 0.89, {{"calories", 100}, {"protein", 8}, {"fat", 2.5}, {"sodium", 125}}},
    {"ice cream", 1.59, {{"calories", 330}, {"protein", 8}, {"fat", 10}, {"sodium", 180}}},
  };
}

void here() {
  fmt::print("Here\n");
}

bool solveDietMain() {
  fmt::print("Solving diet problem...\n");
  auto nutrients = getNutrients();
  auto foods = getFoods();
  
  // Init gurobi 
  fmt::print("Initializing gurobi environment...\n");
  GRBEnv* env = new GRBEnv();
  GRBModel model = GRBModel(*env);
  model.set(GRB_StringAttr_ModelName, "diet");
  model.set(GRB_IntParam_LogToConsole, 0);
  model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

  fmt::print("Creating decision variables for nutrients...\n");
  // Create decision variable for each nutrient 
  for (auto& nutrient : nutrients) {

    nutrient.var = model.addVar(nutrient.min, nutrient.max, 0, GRB_CONTINUOUS, nutrient.name);
  }
  fmt::print("Creating food decision variables...\n");

  // Create decision variable for each food 
  for (auto& food : foods) {
    food.var = model.addVar(0, GRB_INFINITY, food.cost, GRB_CONTINUOUS, food.name);
  }
  fmt::print("Adding constraints for nutrients...\n");

  // Ensure nutrients values are always within bounds
  for (auto& nutrient : nutrients) {
    model.addConstr(nutrient.var >= nutrient.min, nutrient.name + "_min");
    model.addConstr(nutrient.var <= nutrient.max, nutrient.name + "_max");
  }

  fmt::print("Linking constraints for nutrients and food variables...\n");
  // Link total nutrient value added from all foods to the nutrient var
  for (auto& nutrient : nutrients) {
    fmt::print("Adding constraint for nutrient: {}\n", nutrient.name);
    GRBLinExpr total_nutrient_value = 0;
    for (auto& food : foods) {
      total_nutrient_value += food.getNutrientValueFromName(nutrient.name) * food.var;
    }
    model.addConstr(total_nutrient_value == nutrient.var, nutrient.name);
  }

  // Solve the model 
  model.optimize();

  fmt::print("Solution:\n");
  // Print the solution 
  for (auto& nutrient : nutrients) {
    nutrient.show();
  }

  for (auto& food : foods) {
    food.show();
  }

  return true;
}

bool solveDiet() {
  try {
    solveDietMain();
    return true;
  } catch (GRBException e) {
    fmt::print("Error code = {}\n", e.getErrorCode());
    fmt::print("Error message: {}\n", e.getMessage());
    return false;
  } catch (...) {
    fmt::print("Exception during optimization");
    return false;
  }
}

bool solveDietLegacy()
{
  GRBEnv* env = 0;
  GRBVar* nutrition = 0;
  GRBVar* buy = 0;

  try
  {

    // Nutrition guidelines, based on
    // USDA Dietary Guidelines for Americans, 2005
    // http://www.health.gov/DietaryGuidelines/dga2005/
    const int nCategories = 4;
    string Categories[] =
      { "calories", "protein", "fat", "sodium" };
    double minNutrition[] = { 1800, 91, 0, 0 };
    double maxNutrition[] = { 2200, GRB_INFINITY, 65, 1779 };

    // Set of foods
    const int nFoods = 9;
    string Foods[] =
      { "hamburger", "chicken", "hot dog", "fries",
        "macaroni", "pizza", "salad", "milk", "ice cream" };
    double cost[] =
      { 2.49, 2.89, 1.50, 1.89, 2.09, 1.99, 2.49, 0.89, 1.59 };

    // Nutrition values for the foods
    double nutritionValues[][nCategories] = {
                      { 410, 24, 26, 730 },    // hamburger
                      { 420, 32, 10, 1190 },   // chicken
                      { 560, 20, 32, 1800 },   // hot dog
                      { 380, 4, 19, 270 },     // fries
                      { 320, 12, 10, 930 },    // macaroni
                      { 320, 15, 12, 820 },    // pizza
                      { 320, 31, 12, 1230 },   // salad
                      { 100, 8, 2.5, 125 },    // milk
                      { 330, 8, 10, 180 }      // ice cream
                    };

    // Model
    env = new GRBEnv();
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "diet");
    // set verbose to false 
    model.set(GRB_IntParam_LogToConsole, 0);

    // Create decision variables for the nutrition information,
    // which we limit via bounds
    nutrition = model.addVars(minNutrition, maxNutrition, 0, 0,
                              Categories, nCategories);

    // Create decision variables for the foods to buy
    //
    // Note: For each decision variable we add the objective coefficient
    //       with the creation of the variable.
    buy = model.addVars(0, 0, cost, 0, Foods, nFoods);

    // The objective is to minimize the costs
    //
    // Note: The objective coefficients are set during the creation of
    //       the decision variables above.
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    // Nutrition constraints
    for (int i = 0; i < nCategories; ++i)
    {
      GRBLinExpr ntot = 0;
      for (int j = 0; j < nFoods; ++j)
      {
        ntot += nutritionValues[j][i] * buy[j];
      }
      model.addConstr(ntot == nutrition[i], Categories[i]);
    }

    // Solve
    model.optimize();
    printSolution(model, nCategories, nFoods, buy, nutrition);

    cout << "\nAdding constraint: at most 6 servings of dairy" << endl;
    model.addConstr(buy[7] + buy[8] <= 6.0, "limit_dairy");

    // Solve
    model.optimize();
    printSolution(model, nCategories, nFoods, buy, nutrition);

    // Clean up memory
    delete[] nutrition;
    delete[] buy;
    delete env;
    
    return true;
  }
  catch (GRBException e)
  {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
    
    // Clean up memory in case of exception
    delete[] nutrition;
    delete[] buy;
    delete env;
    
    return false;
  }
  catch (...)
  {
    cout << "Exception during optimization" << endl;
    
    // Clean up memory in case of exception
    delete[] nutrition;
    delete[] buy;
    delete env;
    
    return false;
  }
}

void printSolution(GRBModel& model, int nCategories, int nFoods,
                   GRBVar* buy, GRBVar* nutrition)
{
  if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
  {
    cout << "\nCost: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
    cout << "\nBuy:" << endl;
    for (int j = 0; j < nFoods; ++j)
    {
      if (buy[j].get(GRB_DoubleAttr_X) > 0.0001)
      {
        cout << buy[j].get(GRB_StringAttr_VarName) << " " <<
        buy[j].get(GRB_DoubleAttr_X) << endl;
      }
    }
    cout << "\nNutrition:" << endl;
    for (int i = 0; i < nCategories; ++i)
    {
      cout << nutrition[i].get(GRB_StringAttr_VarName) << " " <<
      nutrition[i].get(GRB_DoubleAttr_X) << endl;
    }
  }
  else
  {
    cout << "No solution" << endl;
  }
}