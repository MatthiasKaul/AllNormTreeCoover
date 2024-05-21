#pragma once

#include <vector>
#include <string>
#include "disjointSet.hpp"
struct neighbour {
  size_t nb;
  size_t cost;
  bool operator<(const neighbour& other) const{
    return cost < other.cost;
  }
  bool operator>(const neighbour& other) const{
    return cost > other.cost;
  }
};

class RootedForest;

class Graph {
private:
  std::vector<std::vector< neighbour >> adjList;
  size_t _n;
public:
  Graph(size_t n);
  virtual ~Graph () = default;
  void addEdge(size_t v, size_t w, size_t cost);
  Graph mst();
  size_t maxEdge() const;
  RootedForest root();
  RootedForest rootWithCutoff(size_t R);
  RootedForest partition(int k);
  void test(int k);
  std::pair<Graph, std::vector<size_t>> remap();
};

Graph readFile(const std::string&& fname);

class RootedForest {
private:
  std::vector<std::vector< neighbour >> childList;
  std::vector< neighbour > parents;
  std::vector<size_t> roots;
  std::vector<size_t> subtreeWeights;
  size_t _n;
  size_t _m;

public:
  RootedForest (size_t n);
  virtual ~RootedForest () = default;
  void addEdge(size_t v, size_t w, size_t cost);
  void addRoot(size_t r){roots.push_back(r); parents[r].nb = std::numeric_limits<size_t>::max(); }
  void diagnose();
  int componentEstimate(int R);
  void computeSubtreeWeights();
  size_t totalWeight();
  bool splitOff(size_t r, int R, float Aggression);
  void cutOffAbove(size_t r);
  void removeEdge(size_t v, size_t w);
  bool tryPartition(int R, int k, float aggression);
  bool findAndCutSubtree(size_t r, int lb, int ub);
  void EvenPartition(size_t r);
  void splitTo(int k);
  disjointSets getPartitionStructure();
};
