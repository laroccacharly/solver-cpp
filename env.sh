alias init="vcpkg new --application"
alias addfmt="vcpkg add port fmt"

# Gurobi setup
export GUROBI_HOME=/Library/gurobi1200/macos_universal2
echo "GUROBI_HOME set to: $GUROBI_HOME"
# MPS files 
export MPS_FILES_DIR="$HOME/.miplib_benchmark/mps_files"
alias setup_miplib="uv run setup_miplib.py"

export PROJECT_NAME="solver-cpp"
alias preset='cmake --preset=vcpkg' 
alias build='cmake --build build'
alias run="./build/$PROJECT_NAME"
alias brun="build && run"
alias clean='rm -rf build'
alias r="build && run -a"


# python scripts 
alias showdb="uv run show_sqlite.py"
alias ui="uv run streamlit run ui.py"
alias select="uv run python -m src.pysolver.make_instance_selection"
alias metrics="uv run python -m src.pysolver.compute_metrics"
alias instances="uv run instances_with_binary_variables.py"


# Run all 
alias runall="r syncdb && r seed && r grb_only && select && r warm_start && r lns && metrics"


# Docker 
export APP_NAME="solver-cpp"
export IMAGE_NAME="$APP_NAME-image"
export CONTAINER_NAME="$APP_NAME-container"
export DOCKER_DEFAULT_PLATFORM=linux/amd64

alias dbuild="docker build -t $IMAGE_NAME ."
alias drun="echo "http://localhost:8501" && docker run --name $CONTAINER_NAME -p 8501:8501 -v $(pwd)/src:/app/src $IMAGE_NAME"
alias start="echo "http://localhost:8501" && docker start -a $CONTAINER_NAME"
alias stop="docker stop $CONTAINER_NAME"

# Deploy 
alias launch="fly launch --name $APP_NAME --no-deploy"
alias deploy="fly deploy"