#ifndef MIP_H
#define MIP_H

#include <vector>

#include "Route.h"
#include "Params.h"

class MIP
{
  private:
    std::vector<Rute>& routes;
    Params* params;

  public:
    MIP(std::vector<Rute>&, Params*);
    void work();
};

#endif
