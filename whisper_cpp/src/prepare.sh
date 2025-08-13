#!/bin/sh

model=""
if [ "$#" -lt 1 ]; then
  model="base"
else
  model="$1"
fi
./download-ggml-model.sh "$model"
cmake -B build
cmake --build build -j --config Release
