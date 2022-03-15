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
#include <utility>
#include "google/protobuf/text_format.h"
#include "benchmarker.h"


namespace math_opt_benchmark {

Benchmarker::Benchmarker(std::vector<std::string> proto_contents, bool is_mip)
  : proto_contents_(std::move(proto_contents)),
    solvers_(is_mip ? mip_solvers : lp_solvers) {
  for (int i = 0; i < solvers_.size(); i++) {
    solver_times_.emplace_back(proto_contents_.size());
  }
}

void Benchmarker::SolveAll() {
  for (int i = 0; i < proto_contents_.size(); i++) {
    const std::string& contents = proto_contents_[i];
    std::unique_ptr<BenchmarkInstance> instance = LoadInstanceFromString(contents);
    const operations_research::math_opt::ModelProto& initial_model = instance->initial_model();

    absl::StatusOr<std::unique_ptr<operations_research::math_opt::Model>> new_model =
        operations_research::math_opt::Model::FromModelProto(initial_model).value();

    for (int j = 0; j < solvers_.size(); j++) {
      operations_research::math_opt::SolverType solver_type = solvers_[j];
      absl::Duration solve_time = SolveModel(instance, new_model.value(), solver_type);
      solver_times_[j][i] = solve_time;
      // Remove the added cuts - don't want to call each solver between cuts in case it harms caching
      // There might be a better way to do this
      new_model = operations_research::math_opt::Model::FromModelProto(initial_model).value();
    }
  }
}

absl::Duration Benchmarker::SolveModel(std::unique_ptr<BenchmarkInstance> &instance,
                                       std::unique_ptr<operations_research::math_opt::Model> &model,
                                       operations_research::math_opt::SolverType solver_type) {
  absl::Time start_t = absl::Now();
  std::unique_ptr<operations_research::math_opt::IncrementalSolver> solver = operations_research::math_opt::IncrementalSolver::New(*model, solver_type).value();

  absl::StatusOr<operations_research::math_opt::SolveResult> result = solver->Solve();
  CHECK_EQ(result.value().termination.reason, operations_research::math_opt::TerminationReason::kOptimal)
    << result.value().termination.detail;

  // TODO which solve_stats do we want?

  double obj = result.value().objective_value();
  CHECK_NEAR(obj, instance->objectives(0), 0.0001);

  absl::Duration elapsed = absl::ZeroDuration();
  for (int i = 0; i < instance->model_updates_size(); i++) {
    CHECK_OK(model->ApplyUpdateProto(instance->model_updates(i)));
    absl::StatusOr<operations_research::math_opt::SolveResult> solution = solver->Solve();
    CHECK_EQ(solution.value().termination.reason, operations_research::math_opt::TerminationReason::kOptimal)
      << solution.value().termination.detail;
    elapsed += solution.value().solve_stats.solve_time;
    obj = solution.value().objective_value();
    CHECK_NEAR(obj, instance->objectives(i + 1), 0.0001);
  }

  absl::Duration wall_time = absl::Now() - start_t;
  return elapsed;

}

std::vector<std::vector<absl::Duration>> Benchmarker::GetDurations() {
  return solver_times_;
}
const std::vector<operations_research::math_opt::SolverType> Benchmarker::GetSolvers() {
  return solvers_;
}

//
// Helper functions
//

std::unique_ptr<BenchmarkInstance> LoadInstanceFromString(const std::string &contents) {
  BenchmarkInstance new_instance;
  CHECK(google::protobuf::TextFormat::ParseFromString(contents, &new_instance));
  return std::make_unique<BenchmarkInstance>(new_instance);
}

absl::Duration max_t(const std::vector<absl::Duration> &v) {
  absl::Duration max = absl::ZeroDuration();
  for (absl::Duration t : v) {
    if (max < t) {
      max = t;
    }
  }

  return max;
}

absl::Duration average_t(const std::vector<absl::Duration> &v) {
  int64_t sum = 0;
  for (absl::Duration t : v) {
    sum += t/absl::Nanoseconds(1);
  }
  sum /= v.size();
  return absl::Nanoseconds(sum);
}

} // namespace math_opt_benchmark