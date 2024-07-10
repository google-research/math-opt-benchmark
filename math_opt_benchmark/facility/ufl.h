// Copyright 2024 The MathOpt Benchmark Authors.
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

// Models the uncapacitated facility location problem:
//
// min_{x,y} sum_i f_i*y_i + sum_i sum_j c_{ij}*x_{ij}
// s.t.      sum_i x_{ij} = 1       for all j      every customer j is served by
// a facility
//                x_{ij} <= y_{ij}  for all i, j   customers are only served by
//                open facilities x_{ij} >= 0 y_i in {0, 1}
//
// where f_i are the costs to open a facility and c_{ij} is the cost for
// facility i to serve person j

#ifndef MATH_OPT_BENCHMARK_FACILITY_UFL_H_
#define MATH_OPT_BENCHMARK_FACILITY_UFL_H_

#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"

namespace math_opt_benchmark {

struct UFLProblem {
  int num_facilities;
  int num_customers;
  std::vector<double> open_costs;  // Cost to open facility i, f_i
  std::vector<std::vector<double>>
      supply_costs;  // Cost for facility i to serve customer j, c_{ij}
};

struct UFLSolution {
  double objective_value;
  std::vector<double> open_values;  // The facilities that are open (0 or 1)
  std::vector<int>
      supply_values;  // supply_values[i]: which facility supplies customer i
};

class UFLSolver {
 public:
  UFLSolver(operations_research::math_opt::SolverType solver_type,
            const UFLProblem &problem, bool iterative);
  UFLSolution Solve();
  void AddBenderCut(double sum, const std::vector<double> &y_coefficients);
  void EnforceInteger();
  BenchmarkInstance GetModel();

 private:
  operations_research::math_opt::Model model_;
  std::unique_ptr<operations_research::math_opt::IncrementalSolver> solver_;
  std::unique_ptr<operations_research::math_opt::UpdateTracker> update_tracker_;
  std::vector<std::vector<operations_research::math_opt::Variable>>
      supply_vars_;
  std::vector<operations_research::math_opt::Variable> open_vars_;
  operations_research::math_opt::Variable bender_var_;
  BenchmarkInstance instance_;
  bool iterative_;
};

class UFLBenders {
 public:
  explicit UFLBenders(const UFLProblem &problem,
                      operations_research::math_opt::SolverType solver_type =
                          operations_research::math_opt::SolverType::kGurobi);
  UFLSolution Solve();
  BenchmarkInstance GetModel() { return solver_.GetModel(); }

 private:
  // Add benders cuts until optimal
  UFLSolution benders();

  UFLProblem problem_;
  UFLSolver solver_;
  std::vector<std::vector<int>> cost_indices_;
};

/* HELPER FUNCTIONS */

// Reads the UFL problem in ORLIB-cap format from a string
// https://resources.mpi-inf.mpg.de/departments/d1/projects/benchmarks/UflLib/data-format.html
UFLProblem ParseProblem(const std::string &contents);

// Solves the worker problem for a fixed j:
// min_x sum_{ij} c_{ij}*x_{ij}
//  s.t. sum_i x_{ij} = 1
//             x_{ij} <= y*_i   for all i
//
// Given a solution to the master problem y* (which facilities are open) and
// fixing a customer j, determine the optimal x_{ij} indicating the fraction of
// demand facility i fulfills for j
//
// When we call this from UFLBenders, We assume the ys are sorted according to
// costs c_{ij} (costs[i] <= costs[i+1]), so greedily choosing ys[i] before
// ys[i+1] will minimize the cost
std::vector<double> Knapsack(const std::vector<double> &ys);

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_FACILITY_UFL_H_
