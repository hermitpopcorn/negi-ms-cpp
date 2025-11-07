SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
cd "$SCRIPT_DIR" || { echo "Error: Could not change to script directory."; exit 1; }

mkdir -p ./build && cd ./build
cmake --preset vcpkg .. && make
