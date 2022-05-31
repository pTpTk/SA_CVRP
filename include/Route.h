#ifndef ROUTE_H
#define ROUTE_H

#include <vector>

#include "Individual.h"
#include "Params.h"

template<typename T>
using M = std::vector<std::vector<T>>;
template<typename T>
using V = std::vector<T>;

struct Rute
{
    std::vector<int> clients;
    double totalDemand;
    double totalCost;

    Rute()
    : totalDemand(0.),
      totalCost(0.)
    {}
    
    Rute(std::vector<int>& c, double tc)
    : clients(c),
      totalDemand(0.),
      totalCost(tc)
    {}
};

class R
{
  private:
    double eval_r(V<int>&);
  public:
    Params* params;
    std::vector<Rute> routes;

    R(Params*, M<int>&);
};

#endif