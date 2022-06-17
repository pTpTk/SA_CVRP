#!/bin/bash
#SBATCH --partition=ising
#SBATCH --mem-per-cpu=4GB
#SBATCH --time=12:00:00
#SBATCH --mail-type=BEGIN,END
#SBATCH --mail-user=zxu44@ur.rochester.edu
#SBATCH -n 1

module load cmake
module load gcc
cmake --build build
