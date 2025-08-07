#!/bin/bash

echo "args: $@"

cd whisper.cpp
if [ $# -eq 0 ]; then
	echo "Error: quantization method is not provided."
	echo "Usage: $0 <quantization method 1> ... [-r <model: default:base>] "
	exit 1
fi	
qms=()
model="base"
while [ $# -gt 0 ]; do
	echo "curr arg: $1"
	if [[ "$1" == "-m" ]]; then
		echo "equals to -m"
		shift
		model="$1"
		break
	fi
	qms+=("$1")
	shift
done
echo "qms: ${sqm[@]}"

if [ ! -d "quantized_models" ]; then  
	mkdir quantized_models
fi
for qm in "${qms[@]}"; do
	./build/bin/quantize models/$model.bin quantized_models/$model-$qm.bin $qm
done
