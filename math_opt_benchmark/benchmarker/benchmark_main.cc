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
  math_opt::SolverType::kGlop,
  math_opt::SolverType::kGurobi,
//  math_opt::SolverType::kGlpk,
};
const std::vector<math_opt::SolverType> mip_solvers{
    math_opt::SolverType::kGscip,
    math_opt::SolverType::kGurobi,
//    math_opt::SolverType::kGlpk,
};


namespace math_opt_benchmark {

std::unique_ptr<BenchmarkInstance> LoadInstance(const std::string& filename) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string proto_str = buffer.str();
  BenchmarkInstance new_instance;
  CHECK(google::protobuf::TextFormat::ParseFromString(proto_str, &new_instance));
  return std::make_unique<BenchmarkInstance>(new_instance);
}

absl::Duration SolveModel(std::unique_ptr<BenchmarkInstance>& instance, std::unique_ptr<math_opt::Model>& model, math_opt::SolverType solver_type) {
  absl::Time start_t = absl::Now();
  std::unique_ptr<math_opt::IncrementalSolver> solver = math_opt::IncrementalSolver::New(*model, solver_type).value();

  absl::StatusOr<math_opt::SolveResult> result = solver->Solve();
  CHECK_EQ(result.value().termination.reason, math_opt::TerminationReason::kOptimal) << result.value().termination.detail;

  // TODO store solve_stats?

  double obj = result.value().objective_value();
  CHECK_NEAR(obj, instance->objectives(0), 0.005);

  for (int i = 0; i < instance->model_updates_size(); i++) {
    CHECK_OK(model->ApplyUpdateProto(instance->model_updates(i)));
    absl::StatusOr<math_opt::SolveResult> solution = solver->Solve();
    CHECK_EQ(solution.value().termination.reason, math_opt::TerminationReason::kOptimal) << solution.value().termination.detail;
    absl::Duration t = solution.value().solve_stats.solve_time;
    obj = solution.value().objective_value();
    CHECK_NEAR(obj, instance->objectives(i+1), 0.005);
  }

  absl::Duration elapsed = absl::Now() - start_t;

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
  for (const std::string& filename : proto_filenames) {
    std::unique_ptr<BenchmarkInstance> instance = LoadInstance(filename);
    const math_opt::ModelProto& initial_model = instance->initial_model();

    for (int i = 0; i < solvers.size(); i++) {
    math_opt::SolverType solver_type = solvers[i];
    absl::StatusOr<std::unique_ptr<math_opt::Model>> new_model = math_opt::Model::FromModelProto(initial_model).value();
    absl::Duration solve_time = SolveModel(instance, new_model.value(), solver_type);
    solve_times[i].push_back(solve_time);
    }
  }

  // TODO figure out what to print
  for (int i = 0; i < solvers.size(); i++) {
    std::cout << std::left << std::setw(20) << solvers[i] << absl::FormatDuration(average_t(solve_times[i]));
    std::cout << std::endl;
  }

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
