#!/bin/bash

module load gcc/11.2

if [ -f "./main" ]; then
    rm main
fi
c++ main.cpp -o main

paths=$(ls "./maps" | sort)
paths_array=($paths)

log_path="./logs/$(date +%Y%m%d%H%M%S)_all.log"

for map_path in ${paths_array[@]}; do
    ./PreliminaryJudge ./main -m "./maps/$map_path" -s 0 >> $log_path
done


echo "---------------------------------------" >> $log_path

cat ./main.cpp >> $log_path


#./PreliminaryJudge ./main -m $map_path -s 0 >> $log_path
#
#./replayer/CodeCraft_2024_Replay.x86_64

wait
