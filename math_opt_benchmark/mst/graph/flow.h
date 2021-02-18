//
// Created by Jeremy Weiss on 12/9/20.
//

#ifndef MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_MST_GRAPH_FLOW_H_
#define MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_MST_GRAPH_FLOW_H_

#include "ortools/graph/max_flow.h"

constexpr double kTolerance = 1e-5;
constexpr int kScale = 1 / kTolerance;
constexpr double kMaxCap = INT_MAX / (2*kScale);

class FlowSolver {
 public:
  FlowSolver() = default;

  void add(int start, int end, double capacity) {
    flow_.AddArcWithCapacity(start, end, (int) (capacity * kScale));
  }

  void assertOpt() {
    n_ = flow_.NumNodes();
    CHECK_EQ(flow_.Solve(n_ - 2, n_ - 1), flow_.OPTIMAL);
  }

  bool generate_solution(std::vector<int>& solution) {
    int max_flow = flow_.OptimalFlow() / kScale;
    std::vector<int> result;
    flow_.GetSourceSideMinCut(&result);
    if (max_flow > n_ && result.size() > 2) {
      for (const int &v : result) {
        if (v != n_ - 2) {
          solution.push_back(v);
        }
      }
      return true;
    }

    return false;
  }

 private:
  operations_research::SimpleMaxFlow flow_;
  int n_ = 0;
};

#endif //MATH_OPT_BENCHMARK_MATH_OPT_BENCHMARK_MST_GRAPH_FLOW_H_
