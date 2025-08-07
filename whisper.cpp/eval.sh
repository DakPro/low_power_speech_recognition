#!/bin/bash

models=()
while [ $# -gt 0 ]; do
	models+=( "$1" )
	shift
done

echo "models: ${models[@]}"

touch report.log
echo "Report on model evaluation. The duration of sample recording is 11s (JFK speech)" > report.log
cd whisper.cpp
echo -n "Building whisper-cli... "
cmake -B build > /dev/null
cmake --build build -j --config Release > /dev/null
echo "whisper-cli build"
base_models=("tiny" "tiny.en" "base" "base.en" "small" "small.en" "medium" "medium.en")
echo "-----------------------------"
echo "-----------------------------" >> ../report.log

is_base_model(){
  	for bm in "${base_models[@]}"; do
		if [[ "$1" =~ ^"${bm}"$ ]]; then
			echo "$1 IS base model"
			return 0
		fi
	done
	echo "$1 is not a base model"
	return 1
}


for model in "${models[@]}"; do
	echo "Model $model" >> ../report.log
	if is_base_model $model; then 
		echo "Starting model $model evaluation"
		if [ ! -f models/$model.bin ]; then
			echo -n "Model not found... Downloading $model... "
  			sh ./models/download-ggml-model.sh $model > /dev/null
			mv models/ggml-$model.bin models/$model.bin
  			echo "Downloaded"
		fi
		path="models/$model.bin"
	else
		echo -n "Looking for quantized model $model... "
		if [ ! -f quantized_models/$model.bin ]; then
			echo "Quantized model not found. Skipping..."
			continue
		fi
		path="quantized_models/$model.bin"
		echo "Quantized model found"
	fi
	echo -n "Runtime: " >> ../report.log
	echo -n "Running $model... "
	./build/bin/whisper-cli -m $path -f samples/jfk.wav > tmp.out 2>&1
  	cat tmp.out
  	grep -i -E "total memory|total time" tmp.out >> ../report.log
  	echo "run"
  	echo "----------------------------------" >> ../report.log
  	echo "----------------------------------"
done
