alias init="vcpkg new --application"
export PROJECT_NAME="solver-cpp"
alias addfmt="vcpkg add port fmt"
export PROJECT_NAME="solver-cpp"

# Gurobi setup
export GUROBI_HOME=/Library/gurobi1200/macos_universal2
echo "GUROBI_HOME set to: $GUROBI_HOME"
# MPS files directory
export MPS_FILES_DIR='/Users/charly/.miplib_benchmark/mps_files'

alias preset='cmake --preset=vcpkg' 
alias build='cmake --build build'
alias run='./build/$PROJECT_NAME'
alias brun="build && run"
alias clean='rm -rf build'
alias r="build && run -a"


# python scripts 
alias showdb="uv run show_sqlite.py"
alias ui="uv run streamlit run ui.py"