#include "graph.hpp"
#include <iostream>
#include <random>
#include "timer.hpp"
#include <fstream>
#include <map>
std::random_device dev;
std::mt19937 rng(dev());
#define INVALID std::numeric_limits<size_t>::max()

Graph clusterGridWithObstacles(int n, int obstaclePct, int classes){
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,100);
  size_t id = 0;
  std::vector<std::vector<size_t>>ids(n, std::vector<size_t>(n, 0));

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      if(dist(rng) < obstaclePct ){
        ids[i][j] = INVALID;
      }else{
        ids[i][j] = id;
        id++;
      }
    }
  }

  Graph G(id);

  for (size_t i = 0; i < n-1; i++) {
    for (size_t j = 0; j < n; j++) {
      if(ids[i][j] == INVALID || ids[i+1][j] == INVALID){
        continue;
      }
      G.addEdge(ids[i][j],ids[i+1][j], 1);
    }
  }

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n-1; j++) {
      if(ids[i][j] == INVALID || ids[i][j+1] == INVALID){
        continue;
      }
      G.addEdge(ids[i][j], ids[i][j+1], 1);
    }
  }

  auto [X, mapping] = G.remap();

  Timer T("TimeToPartition");
  //G = G.remap();
  auto F = X.partition(classes);
  T.log();
  auto partition = F.getPartitionStructure();

  std::string fname  =  "grid" + std::to_string(n) + "_p" + std::to_string(obstaclePct) + ".txt";
  std::map<size_t, int> sizes;
  std::ofstream f(fname, std::ios::out);
  if (f.is_open()) {

    f << n << "\n";

    for(auto& line : ids){
      for(auto& id : line){
        if(id != INVALID){
          auto classID = partition.get_parent(mapping[id]);
          if (sizes.count(classID) ){
            sizes[classID] += 1;
          }else{
            sizes[classID] = 1;
          }
          f << partition.get_parent(mapping[id]) << "\n";
        }else{
          f << "x\n";
        }
      }
    }
    f.close();
  }
  std::cout << "Class Sizes: ";
  for(auto& [key, val] : sizes) std::cout << val << " ";
  std::cout << std::endl;
  return G;
}

Graph clusterGridWithWalls(int n, int obstaclePct, int classes){
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,100);
  size_t id = 0;
  int wallOffset = n / 8;
  std::vector<std::vector<size_t>>ids(n, std::vector<size_t>(n, 0));

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      bool isWall = false;
      if(i % wallOffset == 0 && i != 0 && j >= wallOffset && j<= n-wallOffset){
        isWall = true;
      }
      if(j % wallOffset == 0 && j != 0 && i >= wallOffset && i<= n-wallOffset){
        isWall = true;
      }
      bool isDoor = false;
      if(isWall && (i%wallOffset == wallOffset/2 || j%wallOffset == wallOffset/2)  ){
        isDoor = true;
      }

      if( (isWall && !isDoor) || (isDoor && dist(rng) < obstaclePct)){
        ids[i][j] = INVALID;
      }else{
        ids[i][j] = id;
        id++;
      }
    }
  }

  Graph G(id);

  for (size_t i = 0; i < n-1; i++) {
    for (size_t j = 0; j < n; j++) {
      if(ids[i][j] == INVALID || ids[i+1][j] == INVALID){
        continue;
      }
      G.addEdge(ids[i][j],ids[i+1][j], 1);
    }
  }

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n-1; j++) {
      if(ids[i][j] == INVALID || ids[i][j+1] == INVALID){
        continue;
      }
      G.addEdge(ids[i][j], ids[i][j+1], 1);
    }
  }

  auto [X, mapping] = G.remap();

  Timer T("TimeToPartition");
  auto F = X.partition(classes);
  T.log();
  auto partition = F.getPartitionStructure();

  std::string fname  =  "wallGrid" + std::to_string(n) + "_p" + std::to_string(obstaclePct) + ".txt";
  std::map<size_t, int> sizes;
  std::ofstream f(fname, std::ios::out);

  if (f.is_open()) {

    f << n << "\n";

    for(auto& line : ids){
      for(auto& id : line){
        if(id != INVALID){
          auto classID = partition.get_parent(mapping[id]);
          if (sizes.count(classID) ){
            sizes[classID] += 1;
          }else{
            sizes[classID] = 1;
          }
          f << partition.get_parent(mapping[id]) << "\n";
        }else{
          f << "x\n";
        }
      }
    }
    f.close();
  }

  std::cout << "Class Sizes: ";
  for(auto& [key, val] : sizes) std::cout << val << " ";
  std::cout << std::endl;

  return G;
}


Graph generateGrid(int n){
  std::uniform_int_distribution<std::mt19937::result_type> dist(0,100);

  Graph G(n*n);

  for (size_t i = 0; i < n-1; i++) {
    for (size_t j = 0; j < n; j++) {
      G.addEdge(n*i+j,n*(i+1) + j, 1);
    }
  }

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n-1; j++) {
      G.addEdge(n*i+j,n*i + j + 1, 1);
    }
  }


  return G;
}

int main(int argc, char const *argv[]) {
  if(argc != 4){
    std::cout << "Usage: " << argv[0] << " size prob #classes\n";
    return 0;
  }
  int gridSize = std::stoi(argv[1]);
  int blockProb = std::stoi(argv[2]);
  int classes = std::stoi(argv[3]);

  //clusterGridWithObstacles(gridSize, blockProb, classes);
  clusterGridWithWalls(gridSize, blockProb, classes);

  return 0;
}
