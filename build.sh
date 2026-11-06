#!/usr/bin/env bash
# Compiles student_db.cpp (unchanged) to WebAssembly and puts the site in ./public
# Runs automatically on Vercel (see vercel.json). Locally: bash build.sh
set -eo pipefail
cd "$(dirname "$0")"

[ -f .gitignore ] || printf '.emsdk/\npublic/\n' > .gitignore

if ! command -v emcc >/dev/null 2>&1; then
  echo "Emscripten not found. Installing it (one-time, takes a few minutes)..."
  [ -d .emsdk ] || git clone --depth 1 https://github.com/emscripten-core/emsdk.git .emsdk
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