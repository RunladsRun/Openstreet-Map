// graph.h
// < Scott Hong >
//
// Basic graph class using adjacency List.
//
// University of Illinois at Chicago
// CS 251: Spring 2022
// Project #7 - Openstreet Maps
//

#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <utility>
#include <algorithm>

using namespace std;

template<typename VertexT, typename WeightT>

class graph {
  typedef unordered_map<VertexT, set<VertexT> > Edges;
  typedef map< pair<VertexT, VertexT>, WeightT> Weights;
 private:
  Edges EdgeList;
  Weights WeightList;
 public:
  //
  // defualt constructor:
  //
  // Iniciate graph object with empty member variables.
  //
  graph() {
  }

  //
  // clear:
  //
  // clear all vertices, edges, and weights stored in the member variables.
  //
  void clear() {
    EdgeList.clear();
    WeightList.clear();
  }
  //
  // Copy operator (=):
  //
  // copy all vertices, edges, and weights to an another graph object.
  //
  graph& operator=(const graph &other) {
    this->EdgeList = other.EdgeList;
    this->WeightList = other.WeightList;
    return *this;
  }

  //
  // NumVertices:
  //
  // Returns the # of vertices currently in the graph.
  //
  int NumVertices() const {
    return static_cast<int>(this->EdgeList.size());
  }

  //
  // NumEdges:
  //
  // Returns the # of edges currently in the graph.
  //
  int NumEdges() const {
    return static_cast<int>(this->WeightList.size());
  }

  //
  // addVertex:
  //
  // Adds the vertex v to the graph if there's room, and if so
  // returns true.  If the vertex already exists in the graph,
  // then false is returned.
  //
  bool addVertex(VertexT v) {
    if (this->EdgeList.count(v) != 0) {
      return false;
    }
    set<VertexT> newSet;
    this->EdgeList[v] = newSet;
    return true;
  }

  //
  // addEdge:
  //
  // Adds the edge (from, to, weight) to the graph, and returns
  // true. if the edge already exists, the existing edge weight
  // is overwritten with the new edge weight.
  //
  bool addEdge(VertexT from, VertexT to, WeightT weight) {
    if (this->EdgeList.count(from) == 0) {
      return false;
    }
    if (this->EdgeList.count(to) == 0) {
      return false;
    }
    pair<VertexT,VertexT> edge = make_pair(from, to);
    if (this->EdgeList[from].count(to) == 0) {
      this->EdgeList[from].insert(to);
    }
    this->WeightList[edge] = weight;
    return true;
  }

  //
  // getWeight:
  //
  // Returns the weight associated with a given edge.  If
  // the edge exists, the weight is returned via the reference
  // parameter and true is returned.  If the edge does not
  // exist, the weight parameter is unchanged and false is
  // returned.
  //
  bool getWeight(VertexT from, VertexT to, WeightT& weight) const {
    if (this->EdgeList.count(from) == 0) {
      return false;
    }
    if (this->EdgeList.count(to) == 0) {
      return false;
    }
    pair<VertexT,VertexT> edge = make_pair(from, to);
    if (this->WeightList.count(edge) == 0) {
      return false;
    }
    weight = this->WeightList.at(edge);
    return true;
  }

  //
  // neighbors:
  //
  // Returns a set containing the neighbors of v, i.e. all
  // vertices that can be reached from v along one edge.
  // Since a set is returned, the neighbors are returned in
  // sorted order.
  //
  set<VertexT> neighbors(VertexT v) const {
    set<VertexT> S;
    if (this->EdgeList.count(v) == 0) {
      return S;
    }
    S = this->EdgeList.at(v);
    return S;
  }

  //
  // getVertices:
  //
  // Returns a vector containing all the vertices currently in
  // the graph.
  //
  vector<VertexT> getVertices() const {
    vector<VertexT> v;
    for (auto& vt : this->EdgeList) {
      v.push_back(vt.first);
    }
    return v;
  }

  //
  // dump:
  //
  // Dumps the internal state of the graph for debugging purposes.
  //
  void dump(ostream& output) const {
    output << "***************************************************" << endl;
    output << "********************* GRAPH ***********************" << endl;
    output << "**Num vertices: " << this->NumVertices() << endl;
    output << "**Num edges: " << this->NumEdges() << endl;
    output << endl;
    output << "**Vertices:" << endl;
    for (auto& vt : this->EdgeList) {
      output << " " << vt.first << endl;
    }
    output << endl;
    output << "**Edges:" << endl;
    for (auto& vt : this->EdgeList) {
      output << vt.first << ": ";
      for (auto& v : this->neighbors(vt.first)) {
        WeightT weight;
        if (this->getWeight(vt.first, v, weight)) {
          output << "(" << v << ", " << weight << ") ";
        }
      }
      output << endl;
    }
    output << "**************************************************" << endl;
  }
};
