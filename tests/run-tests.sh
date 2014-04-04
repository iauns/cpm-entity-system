#!/bin/bash
cd "$(dirname "$0")"

if [ ! -d ./bin ]; then
  mkdir -p ./bin
fi

# Ensure we fail immediately if any command fails.
set -e

pushd ./bin > /dev/null
  if [[ ! "-n" == $1 ]]; then
    #cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake -DCMAKE_BUILD_TYPE=Release ..
  fi
  make -j4
  #make -j1
  ./entity_system_tests
popd

