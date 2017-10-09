#!/bin/bash
#SBATCH --nodes=1
#SBATCH --sockets-per-node=2 
#SBATCH --cores-per-socket=14 
#SBATCH --partition=short 
#SBATCH --time=0:60:00
#SBATCH --output=/home/khuck/src/phylanx/build-logs/slurm-%A_%a.out

set -e

# Get time as a UNIX timestamp (seconds elapsed since Jan 1, 1970 0:00 UTC)
T="$(date +%s)"

cd $HOME/src/phylanx
source ./gcc.sh
./build-hpx.sh

printf "\nSUCCESS!\n"
T="$(($(date +%s)-T))"
printf "Time to configure and build APEX: %02d hours %02d minutes %02d seconds.\n" "$((T/3600))" "$((T/60%60))" "$((T%60))"

