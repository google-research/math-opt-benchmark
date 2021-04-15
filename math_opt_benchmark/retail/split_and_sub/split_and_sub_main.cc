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
#include "split_and_sub.h"

#include "absl/random/random.h"
#include "absl/flags/flag.h"
#include "ortools/linear_solver/linear_solver.h"
#include "absl/strings/str_join.h"
#include "unordered_map"
#include <cstdio>

//DEFINE_bool(iterative, true);

//bool iteratve_flag() { return FLAGS_iterative; }

namespace math_opt_benchmark {

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  std::cout << name << ": " << absl::StrJoin(vec, ",") << std::endl;
}

void PrintSolution(const SplitAndSubSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  PrintVector(solution.in_assortment, "Solution in assortment");
  PrintVector(solution.must_split, "Solution must split");
}

// Returns vector of indices indicating which y(i,j) is nonzero for each customer (-1 ==> y_i* = 0)
std::vector<int> solve_dual(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem) {
  PrintVector(solution.in_assortment, "x");
  std::vector<int> ys(problem.customer_orders.size(), -1);
  for (int i = 0; i < problem.customer_orders.size(); i++) {
    const std::vector<int>& order = problem.customer_orders[i];
    for (int j : order) {
      if (std::abs(solution.in_assortment[j]) < 0.0001) {
        bool all_zero = true;
        if (problem.substitutions[i].count(j)) {
          std::vector<int> subs = problem.substitutions[i].find(j)->second;
          for (int sub : subs) {
            if (solution.in_assortment[sub] > 0.0001) {
              all_zero = false;
              break;
            }
          }
        }
        if (all_zero) {
          ys[i] = j;
          break;
        }
      }
    }
  }
  return ys;
}

std::vector<double> solve_primal(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem) {
  std::vector<double> ss(problem.customer_orders.size(), 0);
  for (int i = 0; i < problem.customer_orders.size(); i++) {
    const std::vector<int>& order = problem.customer_orders[i];
    for (int j : order) {
      if (!solution.in_assortment[j]) {
        bool all_zero = true;
        if (problem.substitutions[i].count(j)) {
          std::vector<int> subs = problem.substitutions[i].find(j)->second;
          for (int sub : subs) {
            if (solution.in_assortment[sub] > 0.0001) {
              all_zero = false;
              break;
            }
          }
        }
        if (all_zero) {
          ss[i] = 1;
          break;
        }
      }
    }
  }
  return ss;
}

SplitAndSubSolution benders(SplitAndSubSolver& solver, const SplitAndSubProblem& problem) {
  SplitAndSubSolution solution = solver.Solve();
//  double prev_obj = -1.0;
  std::vector<double> xs(solution.in_assortment.size(), -1.0);
//  while (prev_obj != solution.objective_value)
  bool same_assortment = false;
//  while (xs != solution.in_assortment)
  while (!same_assortment) {
//    prev_obj = solution.objective_value;
    same_assortment = true;
    std::copy(solution.in_assortment.begin(), solution.in_assortment.end(), xs.begin());
    std::vector<int> ys = solve_dual(solution, problem);
    PrintVector(ys, "Ys");
    solver.AddBenderCut(ys, problem);
    solution = solver.Solve();
    PrintSolution(solution);
    for (int i = 0; i < xs.size(); i++) {
      if (std::abs(solution.in_assortment[i] - xs[i]) > 0.0001) {
        same_assortment = false;
        break;
      }
    }
  }
  solution.must_split = solve_primal(solution, problem);
  return solution;
}

void SplitAndSubMain() {
  for (int _ = 0; _ < 1000; _++) {
    char str[255];
    sprintf(str, "/Users/jeremyweiss/PycharmProjects/math-opt-benchmark/math_opt_benchmark/retail/split_and_sub/tests/%i.txt", _);
    freopen(str, "w", stdout);
    fprintf(stderr, "%i ", _);
    SplitAndSubProblem problem;
    absl::BitGen bitgen;
    size_t num_items = absl::Uniform(bitgen, 3u, 21u);
    size_t num_customers = absl::Uniform(bitgen, 1u, 11u);
    size_t C = absl::Uniform(bitgen, 1u, 251u);
    std::vector<int> shuffled_indices(num_items);
    std::iota(shuffled_indices.begin(), shuffled_indices.end(), 0);

    for (int i = 0; i < num_items; i++) {
      int rand_weight = absl::Uniform(bitgen, 0, 51);
      problem.weights.push_back(rand_weight);
    }
    printf("items: %zu\t customers: %zu\t capacity: %zu\n", num_items, num_customers, C);
    PrintVector(problem.weights, "Weights");
    for (int i = 0; i < num_customers; i++) {
      size_t max_len = num_items < 7 ? num_items : 7u;
      size_t order_len = absl::Uniform(bitgen, 1u, max_len);
      std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), bitgen);
      std::vector<int> order(shuffled_indices.begin(), shuffled_indices.begin() + order_len);
      problem.customer_orders.push_back(order);
      PrintVector(problem.customer_orders[i], "Order " + std::to_string(i));

      std::unordered_map<int, std::vector<int>> subs;
      for (int item : order) {
//         10% chance to substitute each item
        if (absl::Uniform(bitgen, 0, 1.0) < 0.1) {
          size_t sub_len = absl::Uniform(bitgen, 1u, 3u);
          std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), bitgen);
          std::vector<int> sub_items(shuffled_indices.begin(), shuffled_indices.begin() + sub_len);
          for (int j = 0; j < sub_items.size(); j++) {
            if (sub_items[j]==item) {
              sub_items[j] = shuffled_indices.back();
            }
          }
          subs.insert({item, sub_items});
          printf("%i-%i: ", i, item);
          PrintVector(sub_items, "");
        }
      }

      problem.substitutions.push_back(subs);
    }
    problem.capacity = C;

//  problem.weights = {0,5,32,19,34,14};
//  PrintVector(problem.weights, "Weights");
//  problem.customer_orders.push_back(std::vector<int>({1}));
//  PrintVector(problem.customer_orders[0], "Order 0");
//  problem.substitutions.emplace_back(std::unordered_map<int, std::vector<int>>({{1, {2,5}}}));
//  PrintVector(problem.substitutions[0].find(1)->second, "0-1");
//  problem.customer_orders.push_back(std::vector<int>({1,2, 0}));
//  PrintVector(problem.customer_orders[1], "Order 1");
//  problem.substitutions.push_back(std::unordered_map<int, std::vector<int>>({{2, {4}}}));
//  PrintVector(problem.substitutions[1].find(2)->second, "1-2");
//  problem.capacity = 69;

//    problem.weights = {10};
//    problem.customer_orders.push_back(std::vector({0}));
//    problem.substitutions.emplace_back();
//    problem.capacity = 5;

    printf("Direct:\n");
    SplitAndSubSolver direct_solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem, false);
    SplitAndSubSolution direct_solution = direct_solver.Solve();
    PrintSolution(direct_solution);

    printf("Iterative:\n");
    SplitAndSubSolver solver(operations_research::MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING, problem);
    SplitAndSubSolution solution = benders(solver, problem);
    PrintSolution(solution);

    CHECK_NEAR(direct_solution.objective_value, solution.objective_value, 0.0001);

    /* TODO when does the bender's objective = original objective
     * worker objective might not change between cuts, but solver might loop
     * w + x_1 + x_2 + x_5 + x_1 >= 2
     * w + x_1 + x_1 + x_2 + x_5 >= 2
     *
     */
  }
}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
//  absl::ParseCommandLineFlags(&argc, &argv, true);
//  std::cerr << filename_flag() << std::endl;
  math_opt_benchmark::SplitAndSubMain();
}
