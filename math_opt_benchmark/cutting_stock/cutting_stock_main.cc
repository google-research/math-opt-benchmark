// Copyright 2010-2021 Google LLC
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

// Adapted from math_opt/samples/cutting_stocks.cc

#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include <fstream>

#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ortools/base/logging.h"
#include "ortools/base/status_builder.h"
#include "ortools/base/status_macros.h"
#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"

ABSL_FLAG(std::string, out_dir, "", "Directory to save protos");
ABSL_FLAG(std::string, data, "", "Path to the data file for a single instance");

namespace math_opt_benchmark {

namespace math_opt = operations_research::math_opt;
constexpr double kInf = std::numeric_limits<double>::infinity();

// piece_sizes and piece_demands must have equal length.
// every piece must have 0 < size <= board_length.
// every piece must have demand > 0.
struct CuttingStockInstance {
  std::vector<int> piece_sizes;
  std::vector<int> piece_demands;
  int board_length;
};

// pieces and quantity must have equal size.
// Defined for a related CuttingStockInstance, the total length all pieces
// weighted by their quantity must not exceed board_length.
struct Configuration {
  std::vector<int> pieces;
  std::vector<int> quantity;
};

// configurations and quantity must have equal size.
// objective_value is the sum of the vales in quantity (how many total boards
// are used).
// To be feasible, the demand for each piece type must be met by the produced
// configurations.
struct CuttingStockSolution {
  std::vector<Configuration> configurations;
  std::vector<int> quantity;
  int objective_value = 0;
};

// Solves the worker problem.
//
// Solves the problem on finding the configuration (with its objective value) to
// add the to model that will give the greatest improvement in the LP
// relaxation. This is equivalent to a knapsack problem.
absl::StatusOr<std::pair<Configuration, double>> BestConfiguration(
    const std::vector<double>& piece_prices,
    const std::vector<int>& piece_sizes, const int board_size) {
  int num_pieces = piece_prices.size();
  CHECK_EQ(piece_sizes.size(), num_pieces);
  math_opt::Model model("knapsack");
  std::vector<math_opt::Variable> pieces;
  for (int i = 0; i < num_pieces; ++i) {
    pieces.push_back(
        model.AddIntegerVariable(0, kInf, absl::StrCat("item_", i)));
  }
  model.Maximize(math_opt::InnerProduct(pieces, piece_prices));
  model.AddLinearConstraint(math_opt::InnerProduct(pieces, piece_sizes) <=
                            board_size);
  ASSIGN_OR_RETURN(const math_opt::SolveResult solve_result,
                   math_opt::Solve(model, math_opt::SolverType::kCpSat));
  if (solve_result.termination.reason !=
      math_opt::TerminationReason::kOptimal) {
    return util::InvalidArgumentErrorBuilder()
           << "Failed to solve knapsack pricing problem: "
           << solve_result.termination;
  }
  Configuration config;
  for (int i = 0; i < num_pieces; ++i) {
    const int use = static_cast<int>(
        std::round(solve_result.variable_values().at(pieces[i])));
    if (use > 0) {
      config.pieces.push_back(i);
      config.quantity.push_back(use);
    }
  }
  return std::make_pair(config, solve_result.objective_value());
}

// Solves the full cutting stock problem by decomposition.
absl::StatusOr<CuttingStockSolution> SolveCuttingStock(
    const CuttingStockInstance& instance, BenchmarkInstance& benchmark_proto) {
  math_opt::Model model("cutting_stock");
  model.set_minimize();
  const int n = instance.piece_sizes.size();
  std::vector<math_opt::LinearConstraint> demand_met;
  for (int i = 0; i < n; ++i) {
    const int d = instance.piece_demands[i];
    demand_met.push_back(model.AddLinearConstraint(d, d));
  }
  std::vector<std::pair<Configuration, math_opt::Variable>> configs;
  auto add_config = [&](const Configuration& config) {
    const math_opt::Variable v = model.AddContinuousVariable(0.0, kInf);
    model.set_objective_coefficient(v, 1);
    for (int i = 0; i < config.pieces.size(); ++i) {
      const int item = config.pieces[i];
      const int use = config.quantity[i];
      if (use >= 1) {
        model.set_coefficient(demand_met[item], v, use);
      }
    }
    configs.push_back({config, v});
  };

  // To ensure the leader problem is always feasible, begin a configuration for
  // every item that has a single copy of the item.
  for (int i = 0; i < n; ++i) {
    add_config(Configuration{.pieces = {i}, .quantity = {1}});
  }

  std::unique_ptr<math_opt::UpdateTracker> update_tracker = model.NewUpdateTracker();

  ASSIGN_OR_RETURN(auto solver, math_opt::IncrementalSolver::New(
                                    model, math_opt::SolverType::kGlop));
  int pricing_round = 0;
  while (true) {
    ASSIGN_OR_RETURN(math_opt::SolveResult solve_result, solver->Solve());
    if (solve_result.termination.reason !=
        math_opt::TerminationReason::kOptimal) {
      return util::InternalErrorBuilder()
             << "Failed to solve leader LP problem at iteration "
             << pricing_round << " termination: " << solve_result.termination;
    }
    // GLOP always returns a dual solution on optimal
    CHECK(solve_result.has_dual_feasible_solution());
    std::vector<double> prices;
    for (const math_opt::LinearConstraint d : demand_met) {
      prices.push_back(solve_result.dual_values().at(d));
    }
    ASSIGN_OR_RETURN(
        (const auto [config, value]),
        BestConfiguration(prices, instance.piece_sizes, instance.board_length));

    if (!benchmark_proto.objectives_size()) {
      *(benchmark_proto.mutable_initial_model()) = model.ExportModel();
    }
    benchmark_proto.add_objectives(solve_result.objective_value());

    if (value <= 1 + 1e-3) {
      // The LP relaxation is solved, we can stop adding columns.
      break;
    }
    update_tracker->Checkpoint();
    add_config(config);
    *(benchmark_proto.add_model_updates()) = *update_tracker->ExportModelUpdate();
    LOG(INFO) << "round: " << pricing_round
              << " lp objective: " << solve_result.objective_value();
    pricing_round++;
  }
  LOG(INFO) << "Done adding columns, switching to MIP";
  update_tracker->Checkpoint();
  for (const auto& [config, var] : configs) {
    model.set_integer(var);
  }
  ASSIGN_OR_RETURN(const math_opt::SolveResult solve_result,
                   math_opt::Solve(model, math_opt::SolverType::kCpSat));
  if (solve_result.termination.reason !=
      math_opt::TerminationReason::kOptimal) {
    return util::InternalErrorBuilder()
           << "Failed to solve final cutting stock MIP, termination: "
           << solve_result.termination;
  }
  *(benchmark_proto.add_model_updates()) = *update_tracker->ExportModelUpdate();
  benchmark_proto.add_objectives(solve_result.objective_value());
  CuttingStockSolution solution;
  for (const auto& [config, var] : configs) {
    int use =
        static_cast<int>(std::round(solve_result.variable_values().at(var)));
    if (use > 0) {
      solution.configurations.push_back(config);
      solution.quantity.push_back(use);
      solution.objective_value += use;
    }
  }
  return solution;
}

CuttingStockInstance read_instance(const std::string& filename) {
  CuttingStockInstance instance;

  std::ifstream file(filename);
  std::string line;
  std::getline(file, line);
  std::istringstream lineTokens(line);
  std::string tmp;

  // L= [n]
  lineTokens >> tmp;
  lineTokens >> instance.board_length;

  std::getline(file, line);
  lineTokens = std::istringstream(line);
  int n;
  // m= [n]
  lineTokens >> tmp;
  lineTokens >> n;

  instance.piece_demands.resize(n);
  instance.piece_sizes.resize(n);
  for (int i = 0; i < n; i++) {
    std::getline(file, line);
    lineTokens = std::istringstream(line);
    lineTokens >> instance.piece_sizes[i];
    lineTokens >> instance.piece_demands[i];
  }

  return instance;
}

absl::Status RealMain(const std::string& out_dir, const std::string& data) {
  // Data from https://en.wikipedia.org/wiki/Cutting_stock_problem
  CuttingStockInstance instance = read_instance(data);
  BenchmarkInstance benchmark_proto;
  ASSIGN_OR_RETURN(CuttingStockSolution solution, SolveCuttingStock(instance, benchmark_proto));
  std::ofstream f(out_dir + data.substr(data.find_last_of('/')));
  f << benchmark_proto.DebugString();
  f.close();
  return absl::OkStatus();
}

}  // namespace

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  std::string out_dir = absl::GetFlag(FLAGS_out_dir);
  std::string data = absl::GetFlag(FLAGS_data);
  absl::Status result = math_opt_benchmark::RealMain(out_dir, data);
  if (!result.ok()) {
    std::cerr << result;
    return 1;
  }
  return 0;
}
