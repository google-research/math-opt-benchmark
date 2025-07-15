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

#ifndef MATH_OPT_BENCHMARK_BENCHMARKER_BENCHMARKER_H_
#define MATH_OPT_BENCHMARK_BENCHMARKER_BENCHMARKER_H_

#include "absl/time/time.h"
#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"
#include <vector>

namespace math_opt_benchmark {

class SolveStatsData {
public:
  SolveStatsData(operations_research::math_opt::SolveStats solve_stats);
  void Update(operations_research::math_opt::SolveStats solve_stats);
  void Save(const std::vector<std::string> &filenames);
  int Size();

  absl::Duration duration;
  int num_pivots;

private:
  std::vector<operations_research::math_opt::SolveStatsProto> solve_stats_;
};

class Benchmarker {
public:
  Benchmarker(std::vector<BenchmarkInstance>& proto_contents, const std::vector<operations_research::math_opt::SolverType>& solvers, const std::string& dir);
  void SolveAll();
  // Returns proto to store
  static SolveStatsData SolveModel(const BenchmarkInstance &instance, std::unique_ptr<operations_research::math_opt::Model> &model,
                            operations_research::math_opt::SolverType solver_type);
  void SaveAll(const std::vector<std::string>& filenames);
  void SaveProto(int idx);
  std::vector<std::vector<absl::Duration>> GetDurations();
  std::vector<operations_research::math_opt::SolverType> GetSolvers();

private:
  std::vector<std::vector<SolveStatsData>> solve_data_;
  std::vector<BenchmarkInstance> proto_contents_;
  const std::vector<operations_research::math_opt::SolverType> solvers_;
  std::vector<std::string> solver_names_;
  std::string dir_;
};

std::unique_ptr<BenchmarkInstance> LoadInstanceFromString(const std::string& contents);
std::string solver_to_string(operations_research::math_opt::SolverType solver_type);


absl::Duration max_t(const std::vector<absl::Duration>& v);
absl::Duration average_t(const std::vector<absl::Duration>& v);

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_BENCHMARKER_BENCHMARKER_H_