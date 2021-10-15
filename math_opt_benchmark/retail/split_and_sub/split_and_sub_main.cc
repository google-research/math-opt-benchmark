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

// TODO ADD SUBSTITUTIONS BASED ON FREQUENCIES
// NORMALIZE TO MAKE LARGEST COEFFICIENT 1
// 10-6 removed conservatively (make constraint looser by plugging in {0,1} for variable

// --alsologtostderr

#include <cstdlib>
#include <fstream>
#include "split_and_sub.h"

#include "absl/random/random.h"
#include "absl/flags/flag.h"
#include "absl/time/time.h"
#include "absl/strings/str_join.h"
#include "unordered_map"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <cstdio>
#include <fcntl.h>

#include "math_opt_benchmark/proto/dataset.pb.h"

ABSL_FLAG(bool, print_debug, false, "Print debug messages every iteration");
ABSL_FLAG(bool, start_continuous, false, "Only run the continuous relaxation");
ABSL_FLAG(bool, solve_directly, false, "Compare the optimal value to the direct formulation");
ABSL_FLAG(std::string, data_dir, "", "Full path to the directory containing the Instacart protobufs");
ABSL_FLAG(std::string, solver_type, "scip", "Name of the solver to use");

ABSL_FLAG(bool, test_environment, false, "Only solve one instance");

// TODO flag struct



bool print_debug_flag() { return absl::GetFlag(FLAGS_print_debug); }
bool start_continuous_flag() { return absl::GetFlag(FLAGS_start_continuous); }
bool solve_directly_flag() { return absl::GetFlag(FLAGS_solve_directly); }
std::string data_dir_flag() { return absl::GetFlag(FLAGS_data_dir); }
std::string solver_type_flag() { return absl::GetFlag(FLAGS_solver_type); }

bool test_environment_flag() { return absl::GetFlag(FLAGS_test_environment); }

constexpr double kInf = std::numeric_limits<double>::infinity();

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  LOG(INFO) << name << ": " << absl::StrJoin(vec, ",");
}

void PrintSolution(const SplitAndSubSolution& solution) {
  LOG(INFO) << "Solution objective: " << solution.objective_value;
  PrintVector(solution.in_assortment, "Solution in assortment");
  PrintVector(solution.must_split, "Orders which split");
}

// Returns true if all the xs[i] and subs(xs[i]) are 0 in the solution
bool integer_worker_helper(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem, const int customer, const int item) {
  if (solution.in_assortment[item] < 0.0001) {
    // Check if any substitutions are nonzero
    if (problem.substitutions[customer].count(item)) {
      const std::vector<int> &subs = problem.substitutions[customer].find(item)->second;
      for (int sub : subs) {
        if (solution.in_assortment[sub] > 0.5) {
          return false;
        }
      }
    }
    return true;
  }
  return false;
}

// Returns the coefficient of the y[i] in the dual
double continuous_worker_helper(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem, const int customer, const int item) {
  double coefficient = solution.in_assortment[item];
  if (problem.substitutions[customer].count(item)) {
    const std::vector<int> &subs = problem.substitutions[customer].find(item)->second;
    for (int sub : subs) {
      coefficient += solution.in_assortment[sub];
    }
  }

  return 1 - coefficient;
}

// Returns vector of indices indicating which y(i,j) is nonzero for each customer (-1 ==> y_i* = 0)
// The runtime of this function is negligible
std::vector<int> solve_worker(const SplitAndSubSolution& solution, const SplitAndSubProblem& problem, bool is_primal=false) {
  // Changes for solving the continuous dual
  std::vector<int> ys(problem.customer_orders.size(), -1);
  PrintVector(solution.in_assortment, "xs");
  for (int i = 0; i < problem.customer_orders.size(); i++) {
    const std::vector<int>& order = problem.customer_orders[i];
    double max_coefficient = -kInf;
    int max_index;
    for (int j : order) {
      // Relaxation: coefficient of ys[i] is largest so far ==> set ys[i] == 1
      double coefficient = continuous_worker_helper(solution, problem, i, j);
//      LOG(INFO) << i << ' ' << j << ' ' << coefficient;
      if (coefficient > max_coefficient) {
        max_coefficient = coefficient;
        max_index = j;
      }

      // Upper bound on coefficient = 1
      if (std::abs(max_coefficient - 1) < 0.0001) {
        break;
      }

      /* Outdated variant of continuous_worker_helper - might try this again if phase two is slow
        bool all_zero = integer_worker_helper(solution, problem, i, j);
        if (all_zero) {
          int value = is_primal ? 1 : j;
          ys[i] = value;
          break;
        }
      */

    }
    int value = is_primal ? 1 : max_index;
    ys[i] = value;
  }
  PrintVector(ys, "Ys");
  return ys;
}


SplitAndSubSolution benders(SplitAndSubSolver& solver, const SplitAndSubProblem& problem, BenchmarkInstance& model) {
  SplitAndSubSolution solution = solver.Solve();
  model.set_init_objective(solution.objective_value);
  double ub = kInf;
  double lb = 0.0;

  absl::Duration phase_one_total = absl::ZeroDuration();
  absl::Duration phase_two_total = absl::ZeroDuration();

  while (ub - lb > 0.0001) {
    // Could make this into a function if I really use this a lot:
    // time_func(add_to_duration, print_args, function_ptr, function_args)
    absl::Time phase_two_start = absl::Now();
    std::vector<int> ys = solve_worker(solution, problem);
    absl::Duration phase_two_time = absl::Now() - phase_two_start;
    phase_two_total += phase_two_time;
    LOG_IF(INFO, print_debug_flag()) << "Phase 2 completed in " << phase_two_time << " (total: " << phase_two_total << ")";

    ub = ys.size() - std::count(ys.begin(), ys.end(), -1);
    solver.AddBenderCut(ys, problem);
    math_opt::ModelUpdateProto update = solver.GetUpdateProto();
    *(model.add_model_updates()) = update;

    absl::Time phase_one_start = absl::Now();
    solution = solver.Solve();
    absl::Duration phase_one_time = absl::Now() - phase_one_start;
    phase_one_total += phase_one_time;
    LOG_IF(INFO, print_debug_flag()) << "Phase 1 completed in " << phase_one_time << " (total: " << phase_one_total << ")";

    model.add_update_objectives(solution.objective_value);
    lb = solution.objective_value;

    LOG_IF(INFO, print_debug_flag()) << lb << " <= opt <= " << ub;

  }
  solution.must_split = solve_worker(solution, problem, true);
  return solution;
}

void SplitAndSubMain(math_opt::SolverType solver_type) {

  std::string data_dir = data_dir_flag();
  std::cout << data_dir << std::endl;

  const int max_iterations = 30;
  int num_iteartions = test_environment_flag() ? 1 : max_iterations;
  for (int _ = 0; _ < num_iteartions; _++) {
    std::string file_no = "orders" + std::to_string(_) + ".data";
    std::string file_name = data_dir + file_no;
    std::ifstream order_stream(file_name);
    std::stringstream buffer;
    buffer << order_stream.rdbuf();

    std::cout << "***********   " << _ << "/30" << "   ***********" << std::endl;

    OrderDataset orders;
    orders.ParseFromString(buffer.str());
    size_t num_items = orders.nitems();
    size_t num_customers = orders.orders_size();

    SplitAndSubProblem problem;
    absl::BitGen bitgen;

    for (int i = 0; i < num_items; i++) {
      problem.weights.push_back(1);
    }
    for (int i = 0; i < num_customers; i++) {
      std::vector<int> order(orders.orders(i).items().begin(), orders.orders(i).items().end());
      problem.customer_orders.push_back(order);

      std::unordered_map<int, std::vector<int>> subs;
      for (const Substitution& sub : orders.orders(i).subs()) {
        subs.insert({sub.index(), std::vector(sub.sub_idxs().begin(), sub.sub_idxs().end())});
      }

      problem.substitutions.push_back(subs);
    }
    problem.capacity = absl::Uniform(bitgen, num_items*0.5, num_items*1.5);

    math_opt::SolveParametersProto solve_proto;

    LOG_IF(INFO, print_debug_flag()) << "Direct: ";

    SplitAndSubSolver *direct_solver = nullptr;
    SplitAndSubSolution direct_solution;
    if (solve_directly_flag()) {
      direct_solver = new SplitAndSubSolver(solver_type, problem, false, true);
      direct_solution = direct_solver->Solve();
      std::cout << direct_solution.solve_time << std::endl;

      if (print_debug_flag()) {
        direct_solution.must_split = solve_worker(direct_solution, problem, true);
        PrintSolution(direct_solution);
      }
    }

    LOG_IF(INFO, print_debug_flag()) << "Iterative: ";

    SplitAndSubSolver solver(solver_type, problem, true, true);
    BenchmarkInstance model;
    *(model.mutable_initial_model()) = solver.GetModelProto();
    SplitAndSubSolution solution = benders(solver, problem, model);
    if (solve_directly_flag()) {
      CHECK_NEAR(direct_solution.objective_value, solution.objective_value, 0.0001);
    }

    if (print_debug_flag()) {
      solution.must_split = solve_worker(solution, problem, true);
      PrintSolution(solution);
    }
    std::ofstream f("./cart_tests/" + std::to_string(_) + ".txt");
    f << model.DebugString();
    f.close();

  }
}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  google::InitGoogleLogging(argv[0]);
  std::string solver_type = absl::GetFlag(FLAGS_solver_type);
  std::transform(solver_type.begin(), solver_type.end(), solver_type.begin(), [](char c){ return std::tolower(c); });
  math_opt::SolverType solver;
  if (solver_type == "gurobi") {
    solver = math_opt::SOLVER_TYPE_GUROBI;
  } else {
    solver = math_opt::SOLVER_TYPE_GSCIP;
  }
  math_opt_benchmark::SplitAndSubMain(solver);
}
