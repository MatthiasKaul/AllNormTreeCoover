#include "graph.hpp"
#include "disjointSet.hpp"
#include <random>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <limits>


Graph::Graph(size_t n){
  _n = n;
  adjList = std::vector<std::vector< neighbour >>(_n, std::vector<neighbour>());
}

void Graph::addEdge(size_t v, size_t w, size_t cost){
  for( auto& [nb, c] : adjList[v]){
    if(nb == w) return;
  }
  adjList[v].push_back({w,cost});
  adjList[w].push_back({v,cost});
}

RootedForest::RootedForest(size_t n){
  _n = n;
  childList = std::vector<std::vector< neighbour >>(_n, std::vector<neighbour>());
  parents = std::vector< neighbour >(_n, {std::numeric_limits<size_t>::max(),0});
  _m = 0;
}

void RootedForest::addEdge(size_t v, size_t w, size_t cost){
  for( auto& [nb, c] : childList[v]){
    if(nb == w) return;
  }
  childList[v].push_back({w,cost});
  parents[w] = {v,cost};
  _m++;
}

size_t Graph::maxEdge() const {
  size_t rval = 0;
  for(auto& v : adjList){
    for(auto& [w,c] : v){
      if(rval < c) rval = c;
    }
  }
  return rval;
}

size_t RootedForest::totalWeight(){
  size_t rval = 0;
  for(auto& r : roots){
    rval += subtreeWeights[r];
  }
  return rval;
}

void RootedForest::removeEdge(size_t v,size_t w){
  parents[w] = {std::numeric_limits<size_t>::max(), 0};
  int i = 0;
  while(i < childList[v].size()){
    if(childList[v][i].nb == w) break;
    i++;
  }
  if(i == childList[v].size()){
    return;
  }
  childList[v].erase(childList[v].begin()+i);
}

void RootedForest::cutOffAbove(size_t r){

  if(parents[r].nb == std::numeric_limits<size_t>::max()){
    std::cerr << "Incorrect splitOff\n";
    return;
  }

  auto costReduction = subtreeWeights[r] + parents[r].cost;
  auto s = parents[r].nb;

  while (s != std::numeric_limits<size_t>::max()) {
    subtreeWeights[s] -= costReduction;
    s = parents[s].nb;
  }
  removeEdge(parents[r].nb, r);

  return;
}

bool RootedForest::splitOff(size_t r, int R, float aggression){

  while(subtreeWeights[r] > aggression*R){
    DFS:
    for(auto& [w,c] : childList[r]){
      if(subtreeWeights[w] > aggression*R){
        r = w;
        goto DFS;
      }
    }
    break;
  }

  int removedSoFar = 0;
  std::vector<neighbour> cutvertices;
  for(auto& [v,cost] : childList[r]){
    if(removedSoFar < aggression*R){
      removedSoFar += cost + subtreeWeights[v];
      cutvertices.push_back({v,cost});
    }
  }

  for(auto& [v,c] : cutvertices){
    cutOffAbove(v);
  }
  for(int i = 1; i < cutvertices.size(); i++){

    addEdge(cutvertices[0].nb, cutvertices[i].nb, cutvertices[i].cost + cutvertices[0].cost);
    subtreeWeights[cutvertices[0].nb] += cutvertices[i].cost + cutvertices[0].cost;
  }

  addRoot(cutvertices[0].nb);

  return true;
}

bool RootedForest::tryPartition(int R, int k, float aggression){

  int internalEstimate = 0;
  for(auto r : roots){
    internalEstimate += std::floor( (subtreeWeights[r] + aggression*R) / (aggression*R) );
  }
  if (internalEstimate > k) return false; //R much too small

  while (roots.size() < k){
    size_t maxComponent = 0;
    int maxSize = 0;
    for(auto r : roots){
      if(subtreeWeights[r] > maxSize){
        maxSize = subtreeWeights[r];
        maxComponent = r;
      }
    }
    if(maxSize < 2*aggression*R) return true; // all trees are already so small that further splitting off might produce extremely small subtrees. Run separate cleanup later if more components are available
    splitOff(maxComponent, R, aggression);
  }
  for(auto r : roots){
    if (subtreeWeights[r] > aggression*R) return false;
  }
  return true;
}

void Graph::test(int k){
  auto T = mst();

  auto F = T.root();
  F.computeSubtreeWeights();
}

bool reduceToSubcollection(std::vector<neighbour>& comps, int lb, int ub){
  std::sort( comps.begin(), comps.end(), std::greater<neighbour>() );
  std::vector<neighbour> rval;
  int curCost = 0;
  for(auto& [w,c] : comps){
    if( (curCost + c) < ub){
      curCost += c;
      rval.push_back({w,c});
    }
  }
  if(curCost > lb){
    comps = rval;
    return true;
  }else{
    return false;
  }

}

bool RootedForest::findAndCutSubtree(size_t r, int lb, int ub){
  std::vector<size_t> candidates;
  std::queue<size_t> Q;
  Q.push(r);
  while (Q.size()) {
    auto x = Q.front(); Q.pop();
    bool isViable = true;
    for(auto& [w,c] : childList[x]){
      if(subtreeWeights[w] > ub){
        Q.push(w);
        isViable = false;
      }
      if(subtreeWeights[w] <= ub && subtreeWeights[w] >= lb){
        cutOffAbove(w);
        addRoot(w);
        return true;
      }
    }
    if(isViable)
      candidates.push_back(x);
  }

  for(auto x : candidates){
    std::vector<neighbour> components;
    for(auto& [w,c] : childList[x]){
      components.push_back( {w, c + subtreeWeights[w]} );
    }
    if( reduceToSubcollection(components,lb,ub) ){
      for(auto& [w,c] : components){
        cutOffAbove(w);
      }

      for(int i = 1; i < components.size(); i++){
        addEdge(components[0].nb, components[i].nb, components[i].cost + components[0].cost);
        subtreeWeights[components[0].nb] += components[i].cost + components[0].cost;
      }
      addRoot(components[0].nb);

      return true;
    }
  }
  return false;
}

void RootedForest::EvenPartition(size_t r){
  auto size = subtreeWeights[r];
  int deviation = size / 100 + 1;
  while(true){
    if(findAndCutSubtree(r, size/2 - deviation, size/2 + deviation)) break;
    deviation *= 2;
  }
}

void RootedForest::splitTo(int k){
  while(roots.size() < k){
    size_t r;
    int max = 0;
    for(auto x : roots){
      if(subtreeWeights[x] > max){
        r = x;
        max = subtreeWeights[x];
      }
    }
    EvenPartition(r);
  }
}

std::pair<Graph, std::vector<size_t>> Graph::remap(){
  std::vector<size_t> idRemap(_n);
  for (size_t i = 0; i < _n; i++) {
    idRemap[i] = i;
  }
  auto rng = std::default_random_engine {};
  std::shuffle(idRemap.begin(), idRemap.end(), rng);
  Graph G(_n);
  for (size_t i = 0; i < _n; i++) {
    for(auto& [j, c] : adjList[i]){
      if(j < i) continue;
      G.addEdge(idRemap[i], idRemap[j], c);
    }
  }

  std::vector<size_t> idRemapInverse(_n);
  for (size_t i = 0; i < _n; i++) {
    idRemapInverse[idRemap[i]] = i;
  }
  return {G, idRemap };
}

RootedForest Graph::partition(int k){



  std::vector<RootedForest> candidateSolutions;
  std::vector<float> quality;
  auto T = mst();

  auto F = T.root();
  F.computeSubtreeWeights();
  auto total = F.totalWeight();
  float aggression = 2; //safe value
  int lb =  total / k;

  auto ub = total;

  while(ub > lb){
    int mid = (ub+lb) / 2;
    auto CandidateSolution = T.rootWithCutoff(mid);
    CandidateSolution.computeSubtreeWeights();
    if(CandidateSolution.tryPartition(mid, k, aggression)){
      CandidateSolution.splitTo(k);
      F = CandidateSolution;
      ub = mid;
    }else{
      lb = 1+mid;
    }
  }

  return F;
}

struct edge{
  size_t v;
  size_t w;
  size_t cost;
  bool operator<(const edge& other){
    return cost < other.cost;
  }
};

Graph Graph::mst(){
  Graph rval(_n);
  disjointSets sets(_n);
  std::vector<edge> edges;
  for(size_t i = 0; i < _n; i++){
    for(auto& [w,c] : adjList[i]){
      if(w > i)
        edges.push_back({i,w,c});
    }
  }
  std::sort(edges.begin(), edges.end());
  for(auto& [v,w,c] : edges){
    if(sets.is_same(v,w)) continue;
    rval.addEdge(v,w,c);
    sets.merge(v,w);
  }
  return rval;
}

RootedForest Graph::root(){
  return rootWithCutoff(std::numeric_limits<size_t>::max());
}

RootedForest Graph::rootWithCutoff(size_t R){
  std::queue<size_t> toVisit;
  std::vector<bool> visited(_n, false);
  RootedForest T(_n);
  size_t visitedIndex = 0;
  while(visitedIndex < _n){
    while(visitedIndex < _n && visited[visitedIndex] == true){
      visitedIndex++;
    }
    if(visitedIndex < _n){
      toVisit.push(visitedIndex);
      visited[visitedIndex] = true;
      T.addRoot(visitedIndex);
    }
    while(toVisit.size() > 0){
      size_t v = toVisit.front(); toVisit.pop();

      for(auto& [w, cost] : adjList[v]){
        if(cost > R) continue;
        if(visited[w]) continue;
        visited[w] = true;
        toVisit.push(w);
        T.addEdge(v,w,cost);
      }
    }
  }
  return T;
}

Graph readFile(const std::string&& fname){
  std::ifstream f(fname, std::ios::in);
  if (!f.is_open()) {
    return Graph(0);
  }
  std::string line;
  std::getline(f, line);
  std::stringstream header(line);
  int n;
  header >> n;
  Graph G(n);
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    size_t v,w,cost;
    ss >> v >> w >> cost;
    G.addEdge(v,w,cost);
  }
  f.close();
  return G;
}

void RootedForest::diagnose(){
  computeSubtreeWeights();
  std::cout << "The forest has " << roots.size() << " components\n";
  std::cout << "Roots are: ";
  for(auto& r :roots) std::cout << r << " ";
  std::cout << "\n";

  std::cout << "\nWith Cluster Weights:  \n";
  for(auto& r : roots){
    std::cout << subtreeWeights[r] << " ";
  }
  std::cout << std::endl;

}

void RootedForest::computeSubtreeWeights(){
  subtreeWeights = std::vector<size_t>(_n, 0);

  std::vector<bool> processed(_n, false);

  for(auto& r : roots){
    std::stack<size_t> S;
    S.push(r);

    while(S.size() > 0){
      auto v = S.top();
      if(processed[v]){
        S.pop();
        for(auto& [w,c] : childList[v]){
          subtreeWeights[v] += subtreeWeights[w] + c;
        }
      }else{
        for(auto& [w,c] : childList[v]){
          S.push(w);
        }
        processed[v] = true;
      }

    }
  }
}

disjointSets RootedForest::getPartitionStructure(){
  disjointSets rval(_n);
  for (size_t i = 0; i < _n; i++) {
    for(auto& [j,c] : childList[i]){
      rval.merge(i,j);
    }
  }
  return rval;
}
