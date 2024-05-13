#!/bin/bash

if [ -f "./main" ]; then
    rm main
fi
c++ main.cpp -o main

if [ -n "$1" ]; then
    map_path=$1
    map_name=$(basename $map_path)
else
    paths=$(ls "./maps" | sort)
    paths_array=($paths)
    map_name=${paths_array[-1]}
    map_path="./maps/${paths_array[-1]}"
fi

map_name=${map_name%.*}
log_path="./logs/$(date +%Y%m%d%H%M%S)_${map_name}.log"

./PreliminaryJudge ./main -m $map_path -s 0 >> $log_path


echo e "\n\n\n" >> $log_path

cat ./main.cpp >> log_path

./replayer/CodeCraft_2024_Replay.x86_64

wait
