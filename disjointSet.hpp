#pragma once
#include <vector>

class disjointSets {
private:
  std::vector<size_t> parents;
  size_t size;
public:
  size_t get_parent(size_t i);
  bool is_same(size_t i, size_t j);
  void merge(size_t i, size_t j);
  disjointSets (size_t n);
  virtual ~disjointSets () = default;
};
