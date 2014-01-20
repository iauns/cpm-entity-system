#!/bin/bash
cd "$(dirname "$0")"

if [ ! -d ./bin ]; then
  mkdir -p ./bin
fi

# Ensure we fail immediately if any command fails.
set -e

pushd ./bin > /dev/null
  emconfigure cmake -DCMAKE_BUILD_TYPE=Release ..
  #emconfigure cmake -DCMAKE_AR=${HOME}/src/emscripten/emar -DCMAKE_BUILD_TYPE=Debug ..
  VERBOSE=1 emmake make
  node entity_system_tests.js
popd

