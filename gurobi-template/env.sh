export GUROBI_HOME=/Library/gurobi1200/macos_universal2
echo $GUROBI_HOME
export PROJECT_NAME="gurobi-template"
alias mkbuild="cmake -B build"
alias build="cmake --build build"
alias run="./build/${PROJECT_NAME}"