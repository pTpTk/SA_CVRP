#include "List.h"
#include <iostream>

void List::add(LV<int>& Rs){
    for(auto& r : Rs){
        if(!route_list.empty()){
            route_list.remove(r);
        }
        route_list.emplace_back(r);
    }
}

void List::add(M<int>& Rs){
    if(Rs.empty()) return;

    for(auto& r : Rs){
        if(r.empty()) continue;

        if(!route_list.empty()){
            route_list.remove(r);
        }
        route_list.emplace_back(r);
    }
}

M<int> List::out(){
    M<int> outM(route_list.size());
    int i = 0;

    for(auto& r : route_list){
        outM[i++] = r;
    }

    for(auto& r : outM){
        std::cout << "Route: ";
        for(auto& c : r){
            std::cout << c << ' ';
        }
        std::cout << std::endl;
    }

    return outM;
}