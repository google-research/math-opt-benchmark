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

ABSL_FLAG(std::string, instance_dir, "", "Path to directory containing model protos");
ABSL_FLAG(bool, is_mip, true, "Path to directory containing model protos");
ABSL_FLAG(bool, is_single_file, false, "Whether instance_dir points to a directory or a single file");

namespace math_opt = operations_research::math_opt;

namespace math_opt_benchmark {

void BenchmarkMain(const std::vector<std::string>& proto_filenames, bool is_mip) {
  std::vector<std::string> proto_contents(proto_filenames.size());
  for (int i = 0; i < proto_filenames.size(); i++) {
    const std::string &filename = proto_filenames[i];
    CHECK(file::GetContents(filename, &proto_contents[i], file::Defaults()).ok());
  }

  Benchmarker benchmarker(proto_contents, is_mip);
  benchmarker.SolveAll();
  std::vector<std::vector<absl::Duration>> raw_times = benchmarker.GetDurations();
  const std::vector<math_opt::SolverType> &solvers = benchmarker.GetSolvers();

  // TODO figure out what to print
  for (int i = 0; i < solvers.size(); i++) {
    std::cout << std::left << std::setw(20) << solvers[i] << absl::FormatDuration(average_t(raw_times[i])) << std::endl;
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

  math_opt_benchmark::BenchmarkMain(proto_filenames, is_mip);
}
