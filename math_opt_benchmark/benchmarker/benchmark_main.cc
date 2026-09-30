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


#include "ortools/base/file.h"
#include "absl/flags/flag.h"
#include "benchmarker.h"
#include <fstream>

ABSL_FLAG(std::string, instances, "", "Path to directory or file containing model protos");
ABSL_FLAG(std::string, save_dir, "", "Path to directory to store solve results");
ABSL_FLAG(int, start_idx, 0, "Index in sorted directory to start solves from (see numbering of saved protos if solving failed)");
ABSL_FLAG(std::vector<std::string>, solvers, {}, "List of solvers to benchmark");
ABSL_FLAG(bool, print_summary, false, "Print some summary statistics.");


namespace math_opt = operations_research::math_opt;

const std::vector<operations_research::math_opt::SolverType> lp_solvers{
    operations_research::math_opt::SolverType::kGurobi,
    operations_research::math_opt::SolverType::kGlop,
    operations_research::math_opt::SolverType::kGlpk,
};
const std::vector<operations_research::math_opt::SolverType> mip_solvers{
    operations_research::math_opt::SolverType::kGurobi,
    operations_research::math_opt::SolverType::kGscip,
};

namespace math_opt_benchmark {

void BenchmarkMain(const std::vector<std::string>& proto_filenames, const std::string& save_dir,
    std::vector<math_opt::SolverType> solvers, bool print_summary) {
  std::vector<BenchmarkInstance> proto_contents(proto_filenames.size());
  for (int i = 0; i < proto_filenames.size(); i++) {
    const std::string &filename = proto_filenames[i];
    CHECK(file::GetTextProto(filename, &proto_contents[i], file::Defaults()).ok());
  }

  Benchmarker benchmarker(proto_contents, solvers, save_dir);
  benchmarker.SolveAll();
  std::vector<std::vector<absl::Duration>> raw_times = benchmarker.GetDurations();

  if (!save_dir.empty()) {
     benchmarker.SaveAll(proto_filenames);
  }

  if (print_summary) {
    for (int i = 0; i < solvers.size(); i++) {
      std::cout << std::left << std::setw(20) << solvers[i] << absl::FormatDuration(average_t(raw_times[i]))
                << std::endl;
    }
  }

}

} // namespace math_opt_benchmark

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  std::string dir = absl::GetFlag(FLAGS_instances);
  std::string save_dir = absl::GetFlag(FLAGS_save_dir);

  std::vector<math_opt::SolverType> solvers;
  for (auto &solver : absl::GetFlag(FLAGS_solvers)) {
    if (solver == "gurobi") {
      solvers.push_back(math_opt::SolverType::kGurobi);
    } else if (solver == "scip") {
      solvers.push_back(math_opt::SolverType::kGscip);
    }
  }
  std::vector<std::string> proto_filenames;
  if (!std::filesystem::is_directory(dir)) {
    proto_filenames.push_back(dir);
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
      if (!entry.is_directory())
      proto_filenames.push_back(entry.path());
    }
  }

  int start_idx = absl::GetFlag(FLAGS_start_idx);
  std::sort(proto_filenames.begin(), proto_filenames.end());
  std::vector<std::string> proto_filenames_filtered{proto_filenames.begin() + start_idx, proto_filenames.end()};
  bool print_summary = absl::GetFlag(FLAGS_print_summary);

  math_opt_benchmark::BenchmarkMain(proto_filenames, save_dir, solvers, print_summary);
}
