#include <map>
#include <string>
#include <set>
#include <stack>

#include "graphcolor.h"

namespace L2 {

using Graph = std::map<std::string, std::set<std::string>>;

struct Node {
  std::string name;
  std::set<std::string> neighbors;
};

constexpr size_t K = 15;

bool isSpillTemp(const std::string &name) {
  return name.rfind("%SPILL", 0) == 0;
}

// a helper function to get the variable with the most connections, prioritizing <15
std::optional<std::string> selectNode(Graph &graph) {

  std::optional<std::string> simplifyCandidate;
  size_t simplifyDegree = 0;

  std::optional<std::string> spillCandidate;
  size_t spillDegree = 0;

  for (const auto &[node, neighbors] : graph) {

    // only color virtual registers
    if (node.empty() || node[0] != '%') {
      continue;
    }

    size_t degree = neighbors.size();

    // Prefer simplify nodes: degree < K
    if (degree < K) {

      if (!simplifyCandidate.has_value() ||
          degree > simplifyDegree) {

        simplifyCandidate = node;
        simplifyDegree = degree;
      }
    }
    else {

      if (isSpillTemp(node)) {
        continue;
      }

      // potential spill candidate
      if (!spillCandidate.has_value() ||
          degree > spillDegree) {

        spillCandidate = node;
        spillDegree = degree;
      }
    }
  }

  // Always prefer simplify candidates
  if (simplifyCandidate.has_value()) {
    return simplifyCandidate;
  }

  // Otherwise choose spill candidate
  return spillCandidate;
}

void removeNode(Graph &graph, const std::string &node, std::stack<Node> &removed) {
  Node rn{node, graph[node]};
  removed.push(rn);

  for (const auto& neighbor : graph[node]) {

    if (neighbor[0] == '%') { 
      graph[neighbor].erase(node);
    }
  }

  graph.erase(node);
}

ColorResult graphColor(Function &f) {

  std::stack<Node> removed;

  for (const auto& [node, _] : f.graph) {
    // color registers trivially
    if (node[0] != '%') {
      f.colors[node] = node;
    }

  }

  // remove nodes and add them to the stack
  Graph workingGraph;

  for (const auto &[node, neighbors] : f.graph) {

    if (node.empty() || node[0] != '%') {
      continue;
    }

    workingGraph[node] = neighbors;

    // remove callee-saved registers from interference (must handle later)
    std::string calleeSaved[] = {"r12", "r13", "r14", "r15", "rbp", "rbx"};
    for (auto r : calleeSaved) {
      // only remove registers if the variable is not a shift argument
      if (f.shiftVars.find(node) == f.shiftVars.end()) {
        workingGraph[node].erase(r);
      }
    }
  }

  while (auto node = selectNode(workingGraph)) {
    removeNode(workingGraph, node.value(), removed);
  }

  std::string allColors[] = {"rdi", "rsi", "rdx", "rcx", 
                              "r8" , "r9" , "r10", "r11",
                              "rax", "r12", "r13", "r14",
                              "r15", "rbp", "rbx"};

  while (!removed.empty()) {

    bool taken = false;
    Node n = removed.top(); removed.pop();

    for (int i = 0; i < 15; i++) {
      taken = false;
      for (std::string neighbor : n.neighbors) {

        // check if any neighbors have the given color
        if (f.colors.find(neighbor) != f.colors.end() && f.colors[neighbor] == allColors[i]) {
          taken = true;
          break;
        }

      }

      if (!taken) {
        f.colors[n.name] = allColors[i];
        if (i > 8) {
          // this is a callee-saved register, we need to add it to our function signature
          f.calleeSaves.insert(allColors[i]);
        }
        break;
      }
    }

    if (f.colors.find(n.name) == f.colors.end()) {
      // could not match, must spill
      ColorResult result;
      result.success = false;
      result.var = n.name;
      return result;
    }
  }

  // all colors matched
  ColorResult result;
  result.success = true;
  return result;
}

}