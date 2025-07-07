# solver-cpp 

Example repo to demonstrate how to interact with the Gurobi solver to solve MIPLIB instancesin C++. It implements a random Large Neighbourhood Search heuristic and benchmarks it agains the Gurobi baseline. 

- Data are stored on a local sqlite database. 
- Dashboard is built using a streamlit app. 

# Dependencies 
We assume you already installed the following:
 - Gurobi 12
 - vcpkg to manage the c++ environment. 
 - uv to manage the python environment.

# Setup and installation 
All the commands required are available in env.sh. You can run this command to load the helper aliases. 
```
source env.sh 
```

Download the MIPLIB instances: 
```
uv run setup_miplib.py
```
update MPS_FILES_DIR accordingly. 

Build with vcpkg and cmake:
```
cmake --preset=vcpkg
cmake --build build
```

# Usage 
Sync or create the db based on the schema. 
```
run -a syncdb
```

Seed the database with instance data 
```
run -a seed 
````

Run a first set of experiments with gurobi
```
run -a grb_only 
```

Select instances that we are going to experiment with 
```
uv run python -m src.pysolver.make_instance_selection
```

Run warm-start experiment 
```
run -a warm_start 
```

Run lns experiment 

```
run -a lns
```

Compute metrics: 
```
uv run python -m src.pysolver.compute_metrics
```



