#pragma once

#include <list>
#include <vector>

template<typename T>
using V = std::vector<T>;

template<typename T>
using M = std::vector<std::vector<T>>;

template<typename T>
using LV = std::list<std::vector<T>>;

class List
{
  public:
    LV<int> route_list;

    List(){}
    void add(LV<int>&);
    void add(M<int>&);
    M<int> out();
};