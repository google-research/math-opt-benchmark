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

#include "absl/flags/flag.h"
#include "absl/time/time.h"
#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"
#include "google/protobuf/text_format.h"

#define OUT
ABSL_FLAG(std::string, instance_dir, "", "Path to directory containing model protos");
ABSL_FLAG(bool, is_mip, true, "Path to directory containing model protos");
ABSL_FLAG(bool, is_single_file, false, "Whether instance_dir points to a directory or a single file");

namespace math_opt = operations_research::math_opt;

const std::vector<math_opt::SolverType> lp_solvers{
    math_opt::SOLVER_TYPE_GLOP,
    math_opt::SOLVER_TYPE_GUROBI,
};
const std::vector<math_opt::SolverType> mip_solvers{
    math_opt::SOLVER_TYPE_GSCIP,
    math_opt::SOLVER_TYPE_GUROBI,
};


namespace math_opt_benchmark {

absl::Duration RunModel(const std::string& filename, math_opt::SolverType solver_type, OUT double *objective=nullptr) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string proto_str = buffer.str();
  BenchmarkInstance new_model;
  CHECK(google::protobuf::TextFormat::ParseFromString(proto_str, &new_model));
  const math_opt::ModelProto& initial_model = new_model.initial_model();
  const absl::StatusOr<std::unique_ptr<math_opt::Solver>> new_solver = math_opt::Solver::New(
      solver_type, initial_model, math_opt::SolverInitializerProto{});
  std::cout << new_solver.status() << std::endl;

  absl::Time start_t = absl::Now();

  math_opt::SolveParametersProto solve_parameters;
  // Disable presolve for GLOP
  if (solver_type == math_opt::SOLVER_TYPE_GLOP || solver_type == math_opt::SOLVER_TYPE_GSCIP) {
    solve_parameters.mutable_common_parameters()->set_presolve(math_opt::EMPHASIS_LOW);
  }

  solve_parameters.mutable_common_parameters()->set_enable_output(false);
  absl::StatusOr<math_opt::SolveResultProto> new_solution = (*new_solver)->Solve(solve_parameters);
  CHECK_EQ(new_solution->termination_reason(), math_opt::SolveResultProto::OPTIMAL);
  double obj = new_solution.value().primal_solutions(0).objective_value();
  CHECK_NEAR(obj, new_model.objectives(0), 0.0001);

  for (int i = 0; i < new_model.model_updates_size(); i++) {
    CHECK_OK((*new_solver)->Update(new_model.model_updates(i)));
    new_solution = (*new_solver)->Solve(math_opt::SolveParametersProto{});
    CHECK_EQ(new_solution->termination_reason(), math_opt::SolveResultProto::OPTIMAL);
    absl::Duration t = util_time::DecodeGoogleApiProto(new_solution->solve_stats().solve_time()).value();
    obj = (*new_solution).primal_solutions(0).objective_value();
    CHECK_NEAR(obj, new_model.objectives(i+1), 0.0001);
  }

  absl::Duration elapsed = absl::Now() - start_t;
  if (objective) {
    *objective = (*new_solution).primal_solutions(0).objective_value();
  }

  return elapsed;

}

absl::Duration max_t(const std::vector<absl::Duration>& v) {
  absl::Duration max = absl::ZeroDuration();
  for (absl::Duration t : v) {
    if (max < t) {
      max = t;
    }
  }

  return max;
}

absl::Duration average_t(const std::vector<absl::Duration>& v) {
  int64_t sum = 0;
  for (absl::Duration t : v) {
    sum += t / absl::Nanoseconds(1);
  }
  sum /= v.size();
  return absl::Nanoseconds(sum);
}

void BenchmarkMain(const std::vector<std::string>& proto_filenames, const std::vector<math_opt::SolverType>& solvers) {
  std::vector<std::vector<absl::Duration>> solve_times(solvers.size(), std::vector<absl::Duration>(proto_filenames.size()));
  for (int i = 0; i < lp_solvers.size(); i++) {
    math_opt::SolverType solver_type = mip_solvers[i];
    for (const std::string& filename : proto_filenames) {
      absl::Duration solve_time = RunModel(filename, solver_type);
      solve_times[i].push_back(solve_time);
    }
  }

  for (int i = 0; i < solvers.size(); i++) {
    std::string print = std::to_string(solvers[i]) + ": " + absl::FormatDuration(max_t(solve_times[i]));
    std::cout << std::left << std::setw(20) << print;
  }
  std::cout << std::endl;

}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  std::string dir = absl::GetFlag(FLAGS_instance_dir);
  std::vector<std::string> proto_filenames;
  if (absl::GetFlag(FLAGS_is_single_file)) {
    proto_filenames.push_back(dir);
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
      proto_filenames.push_back(entry.path());
    }
  }

  bool is_mip = absl::GetFlag(FLAGS_is_mip);
  const std::vector<math_opt::SolverType> solvers = is_mip ? mip_solvers : lp_solvers;

  math_opt_benchmark::BenchmarkMain(proto_filenames, solvers);
}



// how many simplex pivots per solve
// common parameters
// glop disable presolve
// gurobi glpk glop clp
// save solve stats proto
// what did presolve optimize out