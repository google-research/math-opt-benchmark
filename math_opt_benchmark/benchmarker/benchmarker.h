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

const std::vector<operations_research::math_opt::SolverType> lp_solvers{
    operations_research::math_opt::SolverType::kGurobi,
    operations_research::math_opt::SolverType::kGlop,
    operations_research::math_opt::SolverType::kGlpk,
};
const std::vector<operations_research::math_opt::SolverType> mip_solvers{
    operations_research::math_opt::SolverType::kGurobi,
    operations_research::math_opt::SolverType::kGscip,
};

class Benchmarker {
public:
  Benchmarker(std::vector<std::string> proto_contents, bool is_mip);
  void SolveAll();
  absl::Duration SolveModel(std::unique_ptr<BenchmarkInstance> &instance, std::unique_ptr<operations_research::math_opt::Model> &model,
                            operations_research::math_opt::SolverType solver_type);
  std::vector<std::vector<absl::Duration>> GetDurations();
  const std::vector<operations_research::math_opt::SolverType> GetSolvers();

private:
  std::vector<std::vector<absl::Duration>> solver_times_;
  std::vector<std::string> proto_contents_;
  const std::vector<operations_research::math_opt::SolverType> &solvers_;
};

std::unique_ptr<BenchmarkInstance> LoadInstanceFromString(const std::string& contents);

absl::Duration max_t(const std::vector<absl::Duration>& v);
absl::Duration average_t(const std::vector<absl::Duration>& v);

} // namespace math_opt_benchmark

#endif //MATH_OPT_BENCHMARK_BENCHMARKER_BENCHMARKER_H_