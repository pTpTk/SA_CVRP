.PHONY : all clean

GSET=/home/zxu44/K_graphs/K4096
BEST=11624
#GSET=/scratch/mhuang_lab/ISING_MACHINES/GSET/set/G001
# BEST=2408
# GSET=/scratch/mhuang_lab/ISING_MACHINES/GSET/set/G022
# BEST=13359
# GSET=/scratch/mhuang_lab/ISING_MACHINES/GSET/new_int_full/G081
# BEST=4383
# GSET=/scratch/mhuang_lab/ashar36/chimera_1_2_4
# BEST=0
# GSET=/scratch/mhuang_lab/ISING_MACHINES/GSET/combo_int_full/G001
# BEST=4225
# GSET=/scratch/mhuang_lab/ISING_MACHINES/GSET/combo_int_full/G041
# BEST=0
# GSET=/scratch/mhuang_lab/ashar36/SA\ codes/inagaki_SA-complete-graph-WK2000/WK2000_1.rud
# BEST=33191
INIT=/scratch/mhuang_lab/ISING_MACHINES/INITS/RND/000.txt
# INIT=/scratch/mhuang_lab/ISING_MACHINES/INITS/TEST/G001
# INIT=/scratch/mhuang_lab/ashar36/init_chim.txt

CXX = g++
CXXFLAGS = -std=c++17 -O3 -funroll-loops -Iinclude/
DEBUGFLAGS = -std=c++17 -g -funroll-loops -Iinclude/
TARGET = cvrp

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,src/.%.o, $(SRC))

all : $(TARGET)

src/*.o : include/*.h

$(TARGET) : $(OBJ)
	module unload gcc;
	module load gcc; \
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	module unload gcc;
	module load gcc; \
	./$(TARGET) -g $(GSET) -i $(INIT) -b $(BEST)

src/.%.o : src/%.cpp
	module unload gcc;
	module load gcc; \
	$(CXX) $(CXXFLAGS) -DPARAMS=\"default.hh\" -o $@ -c $<

debug: $(SRC)
	module unload gcc;
	module load gcc; \
	$(CXX) $(DEBUGFLAGS) -DPARAMS=\"default.hh\" -o $@ $^

clean :
	rm -r $(OBJ) $(TARGET) debug

re : clean make
