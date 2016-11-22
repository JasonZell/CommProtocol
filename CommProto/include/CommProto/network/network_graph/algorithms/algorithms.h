/**
  
*/
#ifndef __COMMPROTOCOL_ALGORITHMS_H
#define __COMMPROTOCOL_ALGORITHMS_H

#include <CommProto/architecture/os/include_defines.h>
#include <CommProto/architecture/api.h>
#include <CommProto/tools/standard_comparator.h>
#include <CommProto/network/network_graph/network_graph.h>
#include <CommProto/network/network_graph/network_edge.h>
#include <CommProto/network/network_graph/network_node.h>
#include <functional>
#include <queue>
#include <set>
#include <unordered_map>

namespace comnet {
namespace network {
namespace algorithms {


/**
  BFS implementation to search for a cluster within the network graph.
*/
template<typename ClusterCompare,
         typename Callback = std::function<void()> >
void BFS(NetworkGraph *graph, Node *root, CommsLink &target, Callback handler) {
  ClusterCompare compare;
  std::vector<Node*> visited;
  std::queue<Node*> queue;
  queue.push(root);
  while (!queue.empty()) {
    Node *node = queue.front();
    queue.pop();
    if (std::find(visited.begin(), visited.end(), node) == visited.end()) {
      std::vector<Edge *> edges = node->GetOutgoing();
      if (compare(node->GetCluster(), target)) {
        handler(target);
        break;
      }
      for (int i = 0; i < edges.size(); ++i) {
        Node *dst = edges.at(i)->GetDest();
        queue.push(dst);
      }
    }
  }
}


/**
  Modified Dijkstra Algorithm to calculate the shortest distances from source to
  all nodes in the graph. It is not greedy, which may be a problem as it may reach
  average case big O (|E| + |V| log |V|), where E is the number of edges and V is the numer of vertices,
  which is good case considering the use of binary heap as the queue.

  Implementation is not tested! We need to test later on... Handler is not used yet.
*/
template<typename ClusterCompare,
         typename NodeCompare,
         typename Callback = std::function<void()> >
void Dijkstra(NetworkGraph *graph, Node *source, Callback handler) {
  auto nodes = graph->GetNodes();
  std::unordered_map<int32_t, Node *> visited;
  std::priority_queue<Node *, std::vector<Node *>, NodeCompare> unvisited;
  for (int i = 0; i < nodes.size(); ++i) {
    if (nodes.at(i) != source) {
      nodes.at(i)->SetCost(std::numeric_limits<int32_t>::max);
    }
    unvisited.push(nodes.at(i));
  }
  source->SetCost(0);
  while (!unvisited.empty()) {
    Node *current = unvisited.top();
    unvisited.pop();
    visited[current->GetId()] = current;
    std::vector<Edge *> edges = current->GetOutgoing();
    for (uint32_t i = 0; i < edges.size(); ++i) {
      Edge *edge = edges[i];
      Node *t = edge->GetDest();
      if (visited.find(t->GetId()) == visited.end()) {
        if (current->GetCost() + edge->GetDist() < t->GetCost()) {
          t->SetCost(current->GetCost() + edge->GetDist());
        }
      }
    }
  }
}
} // algorithms
} // network
} // comnet
#endif // __COMMPROTOCOL_ALGORITHMS_H