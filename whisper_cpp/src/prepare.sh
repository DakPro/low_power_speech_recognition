#!/bin/sh

model=""
if [ "$#" -lt 1 ]; then
  model="base"
else
  model="$1"
fi
./models/download-ggml-model.sh "$model"
pwd
cmake -B build
cmake --build build -j --config Release
