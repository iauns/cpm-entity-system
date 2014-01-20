#!/bin/bash
cd "$(dirname "$0")"

if [ ! -d ./bin ]; then
  mkdir -p ./bin
fi

# Ensure we fail immediately if any command fails.
set -e

pushd ./bin > /dev/null
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  #cmake -DCMAKE_BUILD_TYPE=Release ..
  make -j4
  ./entity_system_tests
popd

