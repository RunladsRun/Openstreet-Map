// application.cpp
// <Scott Hong>
//
// University of Illinois at Chicago
// CS 251: Spring 2022
// Project #7 - Openstreet Maps
//
// References:
// TinyXML: https://github.com/leethomason/tinyxml2
// OpenStreetMap: https://www.openstreetmap.org
// OpenStreetMap docs:
//   https://wiki.openstreetmap.org/wiki/Main_Page
//   https://wiki.openstreetmap.org/wiki/Map_Features
//   https://wiki.openstreetmap.org/wiki/Node
//   https://wiki.openstreetmap.org/wiki/Way
//   https://wiki.openstreetmap.org/wiki/Relation
//

#include <iostream>
#include <iomanip>  /*setprecision*/
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <queue>

#include "tinyxml2.h"
#include "dist.h"
#include "osm.h"
#include "graph.h"

using namespace std;
using namespace tinyxml2;

//
// global variable INF: The highest value of double
//
const double INF = numeric_limits<double>::max();

//
// class - prioritize:
//
// class to implement priortize operator for priority queue.
// Compares two pair<long long, double> and return the one
// has bigger double value;
//
class prioritize {
    public:
      bool operator()(const pair<long long, double> p1,
        const pair<long long, double> p2) const {
      return p1.second > p2.second;
      }
};

//
// wayToEdge:
//
// Takes a certain FootwayInfo, and converts its information
// into edges to the graph. Updates the graph by reference.
//
void wayToEdge(graph<long long, double>& g, FootwayInfo footway,
  map<long long, Coordinates>& cods) {
  for (int i = 0;
    i < static_cast<int>(footway.Nodes.size())-1;
    i++) {
    long long ID1 = footway.Nodes[i];
    long long ID2 = footway.Nodes[i+1];
    double distance = distBetween2Points(cods[ID1].Lat,
      cods[ID1].Lon, cods[ID2].Lat, cods[ID2].Lon);
    g.addEdge(ID1, ID2, distance);
    g.addEdge(ID2, ID1, distance);
  }
}


//
// searchBuilding:
//
// Returns BuildingInfo about the building matches
// with user input string by searching Buildings.
//
BuildingInfo searchBuilding(string str,
  vector<BuildingInfo>& Buildings) {
  BuildingInfo building;
  for (auto& b : Buildings) {
    if (str == b.Abbrev) {
      building = b;
      break;
    }
  }
  if (building.Abbrev == "" && building.Fullname == "") {
    for (auto& b : Buildings) {
      if (b.Fullname.find(str) != string::npos) {
        building = b;
        break;
      }
    }
  }
  return building;
}


//
// findMidBuilding:
//
// Returns the BuildingInfo for the building located closest to
// midpoint between two buildings in the parameter.
//
BuildingInfo findMidBuilding(BuildingInfo b1, BuildingInfo b2,
  vector<BuildingInfo>& Buildings, set<string>& found) {
  Coordinates c1 = b1.Coords;
  Coordinates c2 = b2.Coords;
  Coordinates M = centerBetween2Points(c1.Lat, c1.Lon,
  c2.Lat, c2.Lon);
  double min = INF;
  double distance;
  BuildingInfo midBuilding;
  for (auto& b : Buildings) {
    distance = distBetween2Points(M.Lat, M.Lon,
      b.Coords.Lat, b.Coords.Lon);
    if (distance < min && found.count(b.Fullname) == 0) {
      min = distance;
      midBuilding = b;
    }
  }
  return midBuilding;
}

//
// closeID:
//
// Finds the closest node from a certain building,
// then returns as a long long ID.
//
long long closeID(BuildingInfo b, vector<FootwayInfo>& foots,
  map<long long, Coordinates>& coords) {
  double min = INF;
  double distance;
  long long ID = 0;
  for (auto& foot : foots) {
    for (auto& f : foot.Nodes) {
      distance = distBetween2Points(b.Coords.Lat, b.Coords.Lon,
        coords[f].Lat, coords[f].Lon);
      if (distance < min) {
        min = distance;
        ID = f;
      }
    }
  }
  return ID;
}


//
// Dijkstra:
//
// Iterate Dijkstra algorithm to find the shortest path
// from a certain vertex to other vertices in the graph.
// Updates distances list pass by reference,
// and returns prev list.
//
map<long long, long long> Dijkstra(graph<long long, double>& G,
  long long startV, map<long long, double>& distances) {
  map<long long, long long> prev;
  set<long long> visitSet;
  priority_queue<pair<long long, double>,
  vector<pair<long long, double>>,
  prioritize> pq;
  for (long long v : G.getVertices()) {
    distances[v] = INF;
    prev[v] = 0;
    pq.push(make_pair(v, INF));
  }
  distances.at(startV) = 0;
  pq.push(make_pair(startV, 0));
  while (!pq.empty()) {
    pair<long long, double> currentV = pq.top();
    pq.pop();
    if (distances[currentV.first] == INF) {
      break;
    } else if (visitSet.count(currentV.first) > 0) {
      continue;
    } else {
      visitSet.insert(currentV.first);
      for (long long adjV : G.neighbors(currentV.first)) {
        double edgeData = 0;
        G.getWeight(currentV.first, adjV, edgeData);
        double altDistance = distances[currentV.first] + edgeData;
        if (altDistance < distances[adjV]) {
          distances[adjV] = altDistance;
          prev[adjV] = currentV.first;
          pq.push(make_pair(adjV, altDistance));
        }
      }
    }
  }
  return prev;
}


//
// printBuilding:
//
// prints BuildingInfo's full name, and the coordinate.
//
void printBuilding(BuildingInfo b) {
  cout << " " << b.Fullname << endl;
  cout << " (" << b.Coords.Lat << ", ";
  cout << b.Coords.Lon << ")" << endl;
}

//
// printCoord:
//
// prints Coordinate's ID, latitude, and longitude.
//
void printCoord(long long ID, map<long long, Coordinates>& Nodes) {
  cout << " " << ID << endl;
  cout << " (" << Nodes[ID].Lat << ", ";
  cout << Nodes[ID].Lon << ")" << endl;
}

//
// printPath:
//
// prints the shortest path from prev list recursively.
//
void printPath(map<long long, long long>& prev,
  long long& start, long long cur) {
  if (start == cur) {
    cout << "Path: " << start;
    return;
  }
  printPath(prev, start, prev[cur]);
  cout << "->" << cur;
}

//
// closeBuilding (Creative Components):
//
// Takes BuildingInfo as parameter variable, than find the 
// building located closest to that building. If that building
// is not in the vector, add that BuildinInfo to the vector.
//
void closeBuilding(BuildingInfo b,
  vector<BuildingInfo>& Buildings, vector<BuildingInfo>& v) {
  Coordinates c = b.Coords;
  BuildingInfo close;
  double min = INF;
  double distance;
  for (auto& B : Buildings) {
    distance = distBetween2Points(c.Lat, c.Lon,
      B.Coords.Lat, B.Coords.Lon);
    bool exist = false;
    if (distance < min && B.Fullname != b.Fullname) {
      for (unsigned i = 0; i < v.size(); i++) {
        if (v[i].Fullname == B.Fullname) {
          exist = true;
        }
      }
      if (!exist) {
        min = distance;
        close = B;
      }
    }
  }
  v.push_back(close);
}

void application(
    map<long long, Coordinates>& Nodes, vector<FootwayInfo>& Footways,
    vector<BuildingInfo>& Buildings, graph<long long, double>& Graph) {
  string person1Building, person2Building;
  cout << endl;
  cout << "Enter person 1's building (partial name or abbreviation), or #> ";
  getline(cin, person1Building);
  while (person1Building != "#") {
    cout << "Enter person 2's building (partial name or abbreviation)> ";
    getline(cin, person2Building);
    BuildingInfo building1 = searchBuilding(person1Building, Buildings);
    BuildingInfo building2 = searchBuilding(person2Building, Buildings);
    if (building1.Fullname == "" && building1.Abbrev == "") {
      cout << "Person 1's building not found" << endl;
      cout << "\nEnter person 1's building (partial name or abbreviation), or #> ";
    getline(cin, person1Building);
      continue;
    }
    if (building2.Fullname == "" && building2.Abbrev == "") {
      cout << "Person 2's building not found" << endl;
      cout << "\nEnter person 1's building (partial name or abbreviation), or #> ";
    getline(cin, person1Building);
      continue;
    }
    cout << "Person 1's point:" << endl;
    printBuilding(building1);
    cout << "Person 2's point:" << endl;
    printBuilding(building2);
    set<string> found;
    BuildingInfo mBuilding = findMidBuilding(building1,
      building2, Buildings, found);
    cout << "Destination Building:" << endl;
    printBuilding(mBuilding);
    long long ID1 = closeID(building1, Footways, Nodes);
    cout << "Nearest P1 node:" << endl;
    printCoord(ID1, Nodes);
    long long ID2 = closeID(building2, Footways, Nodes);
    cout << "Nearest P2 node:" << endl;
    printCoord(ID2, Nodes);
    long long MidId = closeID(mBuilding, Footways, Nodes);
    cout << "Nearest destination node:" << endl;
    printCoord(MidId, Nodes);
    map<long long, double> distances1;
    map<long long, long long> prev1 = Dijkstra(Graph, ID1, distances1);
    if (distances1[ID2] >= INF) {
      cout << "Sorry, destination unreachable." << endl;
      cout << "\nEnter person 1's building (partial name or abbreviation), or #> ";
      getline(cin, person1Building);
      continue;
    }
    map<long long, double> distances2;
    map<long long, long long> prev2 = Dijkstra(Graph, ID2, distances2);
    int flag = 1;
    while (flag) {
      if (distances1[MidId] >= INF || distances2[MidId] >= INF) {
        cout << "At least one person was unable to reach the destination building. Finding next closest building..." << endl;
        found.insert(mBuilding.Fullname);
        mBuilding = findMidBuilding(building1,
          building2, Buildings, found);
        MidId = closeID(mBuilding, Footways, Nodes);
        cout << "New destination building:" << endl;
        printBuilding(mBuilding);
        cout << "Nearest destination node:" << endl;
        printCoord(MidId, Nodes);
      } else {
        cout << "Person 1's distance to dest: " << distances1[MidId] << " miles" << endl;
        printPath(prev1, ID1, MidId);
        cout << "\n\n";
        cout << "Person 2's distance to dest: " << distances2[MidId] << " miles" << endl;
        printPath(prev2, ID2, MidId);
        cout << endl;
        flag = 0;
      }
    }
    cout << endl;
    cout << "Enter person 1's building (partial name or abbreviation), or #> ";
    getline(cin, person1Building);
  }
}


//
// Cretive components - Closest 5 buildings:
//
// Takes building's name from user's input. If that building exists 
// in the map, then finds other 5 buildings closest to that building.
// (Not the shortest path, physically closest buildings)
// Prints those five buildings' full name list, then let user to 
// decide which building you want to see the shortest path from 
// input building by typing numbers from the list.
//
void creative(
    map<long long, Coordinates>& Nodes, vector<FootwayInfo>& Footways,
    vector<BuildingInfo>& Buildings, graph<long long, double>& Graph) {
  string str;
  cout << "Enter the building (partial name or abbreviation)> ";
  getline(cin, str);
  while (str != "#") {
    BuildingInfo Building = searchBuilding(str, Buildings);
    if (Building.Fullname == "" && Building.Abbrev == "") {
      cout << "The building not found" << endl;
      cout << "\nEnter the building (partial name or abbreviation)> ";
      getline(cin, str);
      continue;
    }
    vector<BuildingInfo> v;
    for (int i = 0; i < 5; i++) {
      closeBuilding(Building, Buildings, v);
    }
    cout << "List of the closest building from ";
    cout << Building.Fullname << ":\n" << endl;
    for (int i = 1; i < 6; i++) {
      cout << i << ". " << v[i-1].Fullname << endl;
    }
    string ans;
    long long ID1 = closeID(Building, Footways, Nodes);
    cout << "\nEnter the number of building to see the shortest path (0 to stop): ";
    getline(cin, ans);
    while (ans != "0") {
      int index = -1;
      if (ans == "1") {
        index = 0;
      } else if (ans == "2") {
        index = 1;
      } else if (ans == "3") {
        index = 2;
      } else if (ans == "4") {
        index = 3;
      } else if (ans == "5") {
        index = 4;
      } else {
        cout << "Wrong input!" << endl;
        cout << "\nEnter the number of building to see the shortest path (0 to stop): ";
        getline(cin, ans);
        continue;
      }
      map<long long, double> distances;
      long long ID2 = closeID(v[index], Footways, Nodes);
      map<long long, long long> prev = Dijkstra(Graph, ID1, distances);
      if (distances[ID2] >= INF) {
        cout << "Sorry, there is no path to destination." << endl;
      } else {
        cout << "Distance to destination: " << distances[ID2] << " miles" << endl;
        printPath(prev, ID1, ID2);
        cout << endl;
      }
      cout << "\nEnter the number of building to see the shortest path (0 to stop): ";
      getline(cin, ans);
    }
    cout << "Enter the building (partial name or abbreviation)> ";
    getline(cin, str);
  }
}


int main() {
  // maps a Node ID to it's coordinates (lat, lon)
  map<long long, Coordinates>  Nodes;
  // info about each footway, in no particular order
  vector<FootwayInfo>          Footways;
  // info about each building, in no particular order
  vector<BuildingInfo>         Buildings;
  XMLDocument                  xmldoc;

  cout << "** Navigating UIC open street map **" << endl;
  cout << endl;
  cout << std::setprecision(8);

  string def_filename = "map.osm";
  string filename;

  cout << "Enter map filename> ";
  getline(cin, filename);

  if (filename == "") {
    filename = def_filename;
  }

  //
  // Load XML-based map file
  //
  if (!LoadOpenStreetMap(filename, xmldoc)) {
    cout << "**Error: unable to load open street map." << endl;
    cout << endl;
    return 0;
  }

  //
  // Read the nodes, which are the various known positions on the map:
  //
  int nodeCount = ReadMapNodes(xmldoc, Nodes);

  //
  // Read the footways, which are the walking paths:
  //
  int footwayCount = ReadFootways(xmldoc, Footways);

  //
  // Read the university buildings:
  //
  int buildingCount = ReadUniversityBuildings(xmldoc, Nodes, Buildings);

  //
  // Stats
  //
  assert(nodeCount == (int)Nodes.size());
  assert(footwayCount == (int)Footways.size());
  assert(buildingCount == (int)Buildings.size());

  cout << endl;
  cout << "# of nodes: " << Nodes.size() << endl;
  cout << "# of footways: " << Footways.size() << endl;
  cout << "# of buildings: " << Buildings.size() << endl;

  graph<long long, double> G;
  for (auto& node : Nodes) {
    G.addVertex(node.first);
  }
  for (auto& footway : Footways) {
    wayToEdge(G, footway, Nodes);
  }
  cout << "# of vertices: " << G.NumVertices() << endl;
  cout << "# of edges: " << G.NumEdges() << endl;
  cout << endl;
  //
  // Menu
  //
  string userInput;
  cout << "Enter \"a\" for the standard application or "
        << "\"c\" for the creative component application> ";
  getline(cin, userInput);
  if (userInput == "a") {
    application(Nodes, Footways, Buildings, G);
  } else if (userInput == "c") {
    creative(Nodes, Footways, Buildings, G);
  }
  cout << "** Done **" << endl;
  return 0;
}
