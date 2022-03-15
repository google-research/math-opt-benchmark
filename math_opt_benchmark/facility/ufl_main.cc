// Copyright 2022 The MathOpt Benchmark Authors.
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

#include "ortools/base/init_google.h"
#include "ortools/base/file.h"
#include "absl/flags/flag.h"
#include "absl/random/random.h"
#include "absl/strings/str_join.h"
#include "math_opt_benchmark/facility/ufl.h"

ABSL_FLAG(std::string, filename, "", "Path to ORLIB problem specification.");
ABSL_FLAG(std::string, out_dir, "./", "Directory to save protos.");
ABSL_FLAG(bool, iterative, true, "Solve iteratively");

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

template <class T>
void PrintVector(std::vector<T> vec, const std::string& name="Vector") {
  std::cout << name << ": " << absl::StrJoin(vec, ",") << std::endl;
}

void PrintSolution(const UFLSolution& solution) {
  std::cout << "Solution objective: " << solution.objective_value << std::endl;
  PrintVector(solution.open_values, "Solution open values");
  PrintVector(solution.supply_values, "Solution supply values");
}

void PrintORLIB(const UFLSolution& solution) {
  for (const int val : solution.supply_values) {
    printf("%i ", val);
  }
  printf("%.5f\n", solution.objective_value);
}

void UFLMain(const std::string& filename, const std::string& out_dir,
             bool iterative) {
  std::string contents;
  CHECK(file::GetContents(filename, &contents, file::Defaults()).ok());
  UFLProblem problem = ParseProblem(contents);
  if (iterative) {
    UFLBenders solver(problem);
    UFLSolution solution = solver.Solve();

    UFLSolver direct_solver(math_opt::SolverType::kGurobi, problem, false);
    UFLSolution direct_solution = direct_solver.Solve();

    std::ofstream f(out_dir + filename.substr(filename.find_last_of('/')));
    f << solver.GetModel().DebugString();
    f.close();
  } else {
    UFLSolver direct_solver(math_opt::SolverType::kGurobi, problem, false);
    UFLSolution direct_solution = direct_solver.Solve();
    PrintORLIB(direct_solution);
  }
}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  InitGoogle(argv[0], &argc, &argv, true);
  std::string filename = absl::GetFlag(FLAGS_filename);
  std::string out_dir = absl::GetFlag(FLAGS_out_dir);
  std::cerr << filename << std::endl;
  bool iterative = absl::GetFlag(FLAGS_iterative);
  math_opt_benchmark::UFLMain(filename, out_dir, iterative);
}
