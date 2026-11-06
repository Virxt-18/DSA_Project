#!/usr/bin/env bash
# Compiles student_db.cpp (unchanged) to WebAssembly and puts the site in ./public
# Runs automatically on Vercel (see vercel.json). Locally: bash build.sh
set -eo pipefail
cd "$(dirname "$0")"

# Make sure build artifacts are never committed (adds missing entries only)
touch .gitignore
for entry in ".emsdk/" "public/"; do
  grep -qxF "$entry" .gitignore || echo "$entry" >> .gitignore
done

if ! command -v emcc >/dev/null 2>&1; then
  echo "Emscripten not found. Installing it (one-time, takes a few minutes)..."
  # Check for the emsdk script itself, not just the folder. If .emsdk exists but
  # is empty or broken (e.g. restored from cache), wipe it and clone again.
  if [ ! -f .emsdk/emsdk ]; then
    rm -rf .emsdk
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git .emsdk
  fi
  ./.emsdk/emsdk install 3.1.74
  ./.emsdk/emsdk activate 3.1.74
  source ./.emsdk/emsdk_env.sh
fi

mkdir -p public
emcc student_db.cpp -std=c++17 -O2 -include cstdlib \
  -sMODULARIZE=1 -sEXPORT_NAME=StudentDB -sEXIT_RUNTIME=1 -sSINGLE_FILE=1 -sENVIRONMENT=web \
  -o public/student_db.js
cp index.html public/index.html
echo "Done. Open public/index.html through a web server, e.g.: python3 -m http.server -d public 8000"