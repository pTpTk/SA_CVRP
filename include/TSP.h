#ifndef __TSP_HH__
#define __TSP_HH__

#include <random>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <cassert>
#include <algorithm>

#include "Individual.h"
#include "Params.h"

template<typename T>
using M = std::vector<std::vector<T>>;
template<typename T>
using V = std::vector<T>;

class TSP 
{
  public:
    V<double> Coeff;
    Params* params;

    V<int> visited_clients;

    M<int> s_in;

    M<int> s_out;

    double beta1 = 3.0;
    double beta0 = 0.01;
    double beta;
    int sweeps;
    double scale;

    TSP(Params*, M<int>&, V<double>&);
    void work();

  private:
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<double> dis{std::uniform_real_distribution<double> (0., 1.)};

    V<int> Fill(int, V<int>&);
    V<double> Dem(V<int>&);
    M<double> Dist(V<int>&);
    // void SA();
    double qubo_ene(M<bool>&);
    double diff(int, int, M<bool>&);
    // void update_out();
    // bool accept(double);
    // void test();

    // bool validate();

};

#endif
