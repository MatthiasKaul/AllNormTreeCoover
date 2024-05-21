#include "disjointSet.hpp"

disjointSets::disjointSets(size_t n){
  parents = std::vector<size_t>(n, 0);
  for (size_t i = 0; i < n; i++) {
    parents[i] = i;
  }
  size = n;
}

size_t disjointSets::get_parent(size_t i){
  while(parents[i] != i)  {
    parents[i] = parents[parents[i]];
    i = parents[i];
  }
  return i;
}

bool disjointSets::is_same(size_t i, size_t j){
  auto pi = get_parent(i);
  auto pj = get_parent(j);
  return pi == pj;
}

void disjointSets::merge(size_t i, size_t j){
  auto pi = get_parent(i);
  auto pj = get_parent(j);
  parents[pi] = pj;
}
