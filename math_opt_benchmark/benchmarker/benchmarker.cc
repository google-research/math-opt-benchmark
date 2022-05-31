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
#include "ortools/base/file.h"


namespace math_opt_benchmark {

//
//  SolveStatsData
//

SolveStatsData::SolveStatsData(operations_research::math_opt::SolveStats solve_stats)
  : duration(solve_stats.solve_time),
    num_pivots(solve_stats.simplex_iterations),
    solve_stats_({solve_stats.ToProto()}) {

}


void SolveStatsData::Update(operations_research::math_opt::SolveStats solve_stats) {
  duration += solve_stats.solve_time;
  num_pivots += solve_stats.simplex_iterations;
  solve_stats_.push_back(solve_stats.ToProto());
}

void SolveStatsData::Save(const std::vector<std::string>& filenames) {
  for (int i = 0; i < filenames.size(); i++) {
    const std::string& filename = filenames[i];
    const operations_research::math_opt::SolveStatsProto proto = solve_stats_[i];
    CHECK_OK(file::SetTextProto(filename, proto, file::Defaults()));
  }
}

int SolveStatsData::Size() {
  return solve_stats_.size();
}

//
//  Benchmarker
//

Benchmarker::Benchmarker(std::vector<BenchmarkInstance> &proto_contents,
    const std::vector<operations_research::math_opt::SolverType>& solvers, const std::string& dir)
  : proto_contents_(std::move(proto_contents)),
    solvers_(solvers),
    dir_(dir) {
  for (operations_research::math_opt::SolverType solver_type : solvers_) {
    solver_names_.push_back(solver_to_string(solver_type));
  }
}

void Benchmarker::SolveAll() {
  for (int i = 0; i < proto_contents_.size(); i++) {
    const BenchmarkInstance& instance = proto_contents_[i];
    const operations_research::math_opt::ModelProto& initial_model = instance.initial_model();

    absl::StatusOr<std::unique_ptr<operations_research::math_opt::Model>> new_model =
        operations_research::math_opt::Model::FromModelProto(initial_model).value();

    solve_data_.emplace_back();
    for (operations_research::math_opt::SolverType solver_type : solvers_) {
      std::unique_ptr<operations_research::math_opt::Model> tmp_model = new_model.value()->Clone();
      SolveStatsData data = SolveModel(instance, tmp_model, solver_type);
      solve_data_[i].push_back(data);
    }
    if (!dir_.empty()) {
      SaveProto(i);
    }
  }
}

SolveStatsData Benchmarker::SolveModel(const BenchmarkInstance& instance,
                                       std::unique_ptr<operations_research::math_opt::Model> &model,
                                       operations_research::math_opt::SolverType solver_type) {
  std::unique_ptr<operations_research::math_opt::IncrementalSolver> solver = operations_research::math_opt::IncrementalSolver::New(*model, solver_type).value();

  absl::StatusOr<operations_research::math_opt::SolveResult> result = solver->Solve();
  CHECK_EQ(result.value().termination.reason, operations_research::math_opt::TerminationReason::kOptimal)
    << result.value().termination.detail;

  double obj = result.value().objective_value();
  CHECK_NEAR(obj, instance.objectives(0), 0.0001);

  SolveStatsData solve_stats(result.value().solve_stats);
  for (int i = 0; i < instance.model_updates_size(); i++) {
    CHECK_OK(model->ApplyUpdateProto(instance.model_updates(i)));
    absl::StatusOr<operations_research::math_opt::SolveResult> solution = solver->Solve();
    CHECK_EQ(solution.value().termination.reason, operations_research::math_opt::TerminationReason::kOptimal)
      << solution.value().termination.detail;
    obj = solution.value().objective_value();
    CHECK_NEAR(obj, instance.objectives(i + 1), 0.0001);
    solve_stats.Update(solution.value().solve_stats);
  }

  return solve_stats;

}

// Renames the output files
void Benchmarker::SaveAll(const std::vector<std::string>& filenames) {
  for (int i = 0; i < filenames.size(); i++) {
    std::string old = dir_ + "/" + std::to_string(i);
    int name_idx = filenames[i].find_last_of('/');
    std::string fnew = dir_ + "/" + filenames[i].substr(name_idx + 1);
    std::rename(old.c_str(), fnew.c_str());
  }
}

void Benchmarker::SaveProto(int idx) {
  std::vector<SolveStatsData>& solve_stats = solve_data_[idx];
  for (int i = 0; i < solver_names_.size(); i++) {
    std::string dir = dir_ + "/" + std::to_string(idx) + "/" + solver_names_[i] + "/";
    std::vector<std::string> filenames;
    filenames.reserve(solve_stats[i].Size());
    std::filesystem::create_directories(dir);
    for (int j = 0; j < solve_stats[i].Size(); j++) {
      filenames.push_back(dir + std::to_string(j));
    }
    solve_stats[i].Save(filenames);
  }
}

std::vector<std::vector<absl::Duration>> Benchmarker::GetDurations() {
  std::vector<std::vector<absl::Duration>> durations;
  for (int i = 0; i < solve_data_.size(); i++) {
    durations.emplace_back();
    for (const SolveStatsData &solve_stats : solve_data_[i]) {
      durations[i].push_back(solve_stats.duration);
    }
  }
  return durations;
}
std::vector<operations_research::math_opt::SolverType> Benchmarker::GetSolvers() {
  return solvers_;
}

//
// Helper functions
//

std::string solver_to_string(operations_research::math_opt::SolverType solver_type) {
  switch (solver_type) {
    case operations_research::math_opt::SolverType::kGscip:
      return "Gscip";
    case operations_research::math_opt::SolverType::kGlop:
      return "Glop";
    case operations_research::math_opt::SolverType::kGurobi:
      return "Gurobi";
    case operations_research::math_opt::SolverType::kGlpk:
      return "Glpk";
    default:
      return "other";
  }
}

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