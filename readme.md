# solver-cpp 

Example repo to demonstrate how to interact with Gurobi to solve MIPLIB instances in C++. It implements a random Large Neighbourhood Search heuristic and benchmarks it against the Gurobi baseline. 

- Data is stored in a local SQLite database. 
- Dashboard is built using a Streamlit app. 

Live app: [solver-cpp.fly.dev](https://solver-cpp.fly.dev)

# Dependencies 
We assume you have already installed the following:
 - Gurobi 12
 - vcpkg to manage the C++ environment. 
 - uv to manage the Python environment.

# Setup and Installation 
All the commands required are available in env.sh. You can run this command to load the helper aliases. 
```
source env.sh 
```

Download the MIPLIB instances: 
```
uv run setup_miplib.py
```
Update MPS_FILES_DIR accordingly. 

Build with vcpkg and cmake:
```
cmake --preset=vcpkg
cmake --build build
```

# Usage 
Sync or create the database based on the schema: 
```
run -a syncdb
```

Seed the database with instance data: 
```
run -a seed 
```

Run a first set of experiments with Gurobi:
```
run -a grb_only 
```

Select instances that we are going to experiment with: 
```
uv run python -m src.pysolver.make_instance_selection
```

Run warm-start experiment: 
```
run -a warm_start 
```

Run LNS experiment:
```
run -a lns
```

Compute metrics: 
```
uv run python -m src.pysolver.compute_metrics
```



