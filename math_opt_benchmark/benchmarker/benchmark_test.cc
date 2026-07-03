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

#include "gtest/gtest.h"
#include "ortools/math_opt/cpp/math_opt.h"
#include "math_opt_benchmark/proto/model.pb.h"
#include "math_opt_benchmark/benchmarker/benchmarker.h"


namespace math_opt_benchmark {
namespace {

namespace math_opt = ::operations_research::math_opt;

TEST(PipelineTest, SmallExample) {
  //    min x
  // s.t.
  BenchmarkInstance instance;
  math_opt::Model model("Benchmark Example");
  std::unique_ptr<math_opt::UpdateTracker> update_tracker = model.NewUpdateTracker();

  model.set_minimize();

  math_opt::Variable var = model.AddContinuousVariable(0.0, 2.0, "x");
  model.set_objective_coefficient(var, 1);
  *(instance.mutable_initial_model()) = model.ExportModel();

  math_opt::SolveArguments solve_args;
  for (int i = 0; i < 2; i++) {
    instance.add_objectives(i);
    update_tracker->Checkpoint();
    const math_opt::LinearConstraint feasible = model.AddLinearConstraint(i + 1, 2.0);
    model.set_coefficient(feasible, var, 1);
    std::optional <math_opt::ModelUpdateProto> update;
    update = update_tracker->ExportModelUpdate();
    if (update.has_value()) {
      *(instance.add_model_updates()) = *update;
    }
  }

  instance.add_objectives(2);

  std::vector<BenchmarkInstance> solve_protos({instance});
  std::vector<math_opt::SolverType> solvers({math_opt::SolverType::kGscip});
  Benchmarker benchmarker(solve_protos, solvers, "");
  // Checks correctness at runtime
  benchmarker.SolveAll();
}

} // namespace
} // namespace math_opt_benchmark
