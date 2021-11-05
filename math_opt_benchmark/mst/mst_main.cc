// Copyright 2020 The MathOpt Benchmark Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include "absl/random/random.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_join.h"
#include "math_opt_benchmark/mst/graph/graph.h"
#include "math_opt_benchmark/mst/mst.h"
#include "ortools/linear_solver/linear_solver.h"

#include "math_opt_benchmark/proto/graph.pb.h"

ABSL_FLAG(std::string, data_dir, "", "Full path to the directory containing the graph protobufs");

constexpr double kTolerance = 1e-5;

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

void PrintSolution(const MSTSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  solution.x_values.print();
}

template <class T>
void PrintVector(std::vector<T> vec) {
  std::cout << "Vector: " << absl::StrJoin(vec, ",") << std::endl;
}

/**
 * Construct graph from edges with non-zero solution values
 * @param problem Graph specification with edges, weights, and number of
 * vertices
 * @param solution Solution values from LP solve routine
 */
Graph toGraph(const MSTProblem& problem, const MSTSolution& solution) {
  std::vector<std::vector<int>> edges(problem.n);
  for (int v1 = 0; v1 < problem.n; v1++) {
    for (int v2 = 0; v2 <= v1; v2++) {
      if (problem.edges.is_set(v1, v2) && std::abs(solution.x_values.get(v1, v2)) > kTolerance) {
        edges[v1].push_back(v2);
        edges[v2].push_back(v1);
      }
    }
  }
  Graph g(std::move(edges));
  return g;
}


MSTSolution iterate_solves(const MSTProblem& problem, MSTSolver& solver) {
  MSTSolution solution = solver.Solve();

  Graph graph = toGraph(problem, solution);
  std::vector<std::vector<int>> invalid = graph.invalid_components(solution.x_values);

  std::vector<std::vector<int>> last;
  std::vector<std::vector<int>> separation_cut(1);
  do {
    while (!invalid.empty() && invalid != last) {
      last = invalid;
      solver.AddConstraints(problem, invalid);
      solution = solver.Solve();
      graph = toGraph(problem, solution);
      invalid = graph.invalid_components(solution.x_values);
    }
    separation_cut[0] = graph.separation_oracle(solution.x_values);
    invalid = separation_cut;
  } while (!separation_cut[0].empty());

  return solution;
}


void MSTMain(std::string graph_dir) {
  for (int _ = 0; _ < 100; _++) {
    std::string file_name = graph_dir + std::to_string(_) + ".pb";
    std::cout << file_name << std::endl;
    std::ifstream order_stream(file_name);
    std::stringstream buffer;
    buffer << order_stream.rdbuf();

    AdjacencyMatrix matrix;
    matrix.ParseFromString(buffer.str());

    MSTProblem problem;
    int n = matrix.vertices_size();
    problem.n = n;
    problem.weights.init(n);
    problem.edges.init(n);
    problem.integer = false;
    for (int i = 0; i < n; i++) {
      for (int j = i; j < n; j++) {
        float weight = matrix.vertices(i).indices(j);
        problem.weights.set(i, j, weight);
        problem.weights.set(j, i, weight);
        if (weight > kTolerance) {
          problem.edges.set(i, j, 1);
          problem.edges.set(j, i, 1);
        }
      }
    }


    MSTSolver solver(operations_research::MPSolver::GUROBI_MIXED_INTEGER_PROGRAMMING, problem);
    MSTSolution solution = iterate_solves(problem, solver);
    solver.EnforceInteger();
    solution = iterate_solves(problem, solver);

    Graph graph = toGraph(problem, solution);
    CHECK_EQ(graph.verify_mst(problem.weights, problem.edges), true);

  }
}

}  // namespace math_opt_benchmark


int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  std::string graph_dir = absl::GetFlag(FLAGS_data_dir);
  math_opt_benchmark::MSTMain(graph_dir);
  return 0;
}