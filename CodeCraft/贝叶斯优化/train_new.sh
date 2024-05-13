#!/bin/bash
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 64

# source /public1/soft/modules/module.sh
# module load anaconda/3-Python3.9.10
# module load gcc/12.2-para
# source activate codecraft

if [ -f "./main" ]; then
    rm main
fi


g++ main.cpp -o main

python3  train.py

wait
