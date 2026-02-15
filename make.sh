SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
cd "$SCRIPT_DIR" || { echo "Error: Could not change to script directory."; exit 1; }

mkdir -p ./build
cmake -B build --preset vcpkg
cmake --build build

pushd src/google-oauth2
cargo build --release
popd
cp src/google-oauth2/target/release/google-oauth2 ./build/

pushd src/clerk-fe
pnpm install && pnpm run build
popd
rm -rf build/clerk-fe; cp -r src/clerk-fe/dist build/clerk-fe
