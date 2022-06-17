#include <cassert>
#include "Route.h"

R::R(Params* params, M<int>& r_out)
: params(params)
{
    for(V<int>& ro : r_out){
        routes.emplace_back(Rute(ro, eval_r(ro)));
    }
}

double R::eval_r(std::vector<int>& r)
{
    double total_cost(0.);
    auto& cost(params->timeCost);

    assert(r.size() > 0);
#ifdef TUNE
    for(auto& i : r) {
        std::cout << i << ' ';
    }
    std::cout << std::endl;
#endif
    for(int i = 1; i < (int)r.size(); ++i){
        total_cost += cost[r[i-1]][r[i]];
    }

    total_cost += cost[0][r[0]] + cost[r.back()][0];

    return total_cost;
}