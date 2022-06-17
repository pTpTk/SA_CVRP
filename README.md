# SA_CVRP
## Compilation
First thing is to load cmake, git, gcc
module load cmake
module load git
module load gcc

Then configure the build
Debug Build (more verbose):
cmake -S. -Bdebug -DBUILD_DEPS:BOOL=ON -DCMAKE_CXX_COMPILER=/software/gcc/11.2.0/b1/bin/g++ -DCMAKE_C_COMPILER=/software/gcc/11.2.0/b1/bin/gcc -DCMAKE_BUILD_TYPE=RelWithDebInfo
Release Build:
cmake -S. -Bbuild -DBUILD_DEPS:BOOL=ON -DCMAKE_CXX_COMPILER=/software/gcc/11.2.0/b1/bin/g++ -DCMAKE_C_COMPILER=/software/gcc/11.2.0/b1/bin/gcc

Finally compile the code
cmake --build <folder name (debug/build)>

Note
* Building with the actual "Debug" build type (or anything less than -O2) leads to errors in one of the dependencies used by the Google or-tools
* CXX compiler has to be set explicitly or cmake would use gcc 4.8

## Runing
There are 5 parameters as mentioned in the overleaf document. The command for running the code is
./cvrp <instancePath> <a0> <a1> <a2> <a3> <b0> [-it nbIter] [-veh nbVehicles]
a0 to b0 corresponds to the coefficients. nbIter is the SA sweep numbers. nbVehicles is the number of vehicles, however it is not needed.

## Problems
The X set can be found in /scrath/mhuang_lab/CVRP/X/ The problem set is based on the paper https://www.sciencedirect.com/science/article/pii/S0377221716306270
