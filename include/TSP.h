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
    M<double> Distance;
    V<double> Demand;
    //V<>
    //M<double> W_2;

    M<bool> spin;

    V<double> Coeff;
    Params* params;
    V<int> nodelist;
    V<int>& r_in;

    M<int> r_out;

    double beta1 = 3.0;
    double beta0 = 0.01;
    double beta;
    int sweeps;
    double scale;

    int graph_size_i;
    int graph_size_j;

    TSP(Params*, V<int>&, V<double>&);
    void work(int);

  private:
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<double> dis{std::uniform_real_distribution<double> (0., 1.)};

    void fill(int);
    void init(int);
    void SA();
    double qubo_ene();
    double diff(int, int);
    void update_out();
    bool accept(double);
    void test();

    bool validate();

};

#endif
