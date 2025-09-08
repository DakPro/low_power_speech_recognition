#!/bin/zsh

for i in {1..5} ; do
  sox sample_90.wav "sample_${i}.wav" trim 0 "${i}"
done
for i in {1..17} ; do
  duration=$((i * 5))
  sox sample_90.wav "sample_${duration}.wav" trim 0 "${duration}"
done