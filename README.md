# MathOpt Benchmark

A set of benchmarks for linear and integer programs as defined using the
MathOpt API in [OR-Tools](https://developers.google.com/optimization).
Unlike the traditional MIPLIB benchmarks, these benchmarks focus on incremental
solves, decompositions, and using callbacks.

This is not an officially supported Google product.

##
Required changes to MathOpt: math_opt.h
```c++
  ModelUpdateProto ExportModelUpdate() const {
    return *(update_tracker_->ExportModelUpdate());
  }
```

## Installation
Bazel will download all the C++ dependencies at build-time (see [WORKSPACE](WORKSPACE) for the full list). Everything was tested using v3.2.0.

Python3 is required to generate random graphs, but it is not needed to run any of the
benchmarks. The only dependency is NumPy.

Gurobi needs to be installed manually. The instructions [from their website](https://www.gurobi.com/documentation/9.1/quickstart_mac/software_installation_guid.html)
should cover everything.

## Creating Random Instances
<strong> Skip this section if you plan to use the included protos. </strong> The retail data is generated
from the Instacart dataset available [here](https://www.kaggle.com/c/instacart-market-basket-analysis)
and requires a Kaggle account to download.

1. Random graphs:
    ```
    cd ./tools/graphs/
    python3 ./generate_graphs.py
    cd ../..
    ```
2. Instacart orders: the Instacart dataset must be in `./tools/retails/` and have its default name.
    ```shell script
    cd ./tools/retails/
    python3 ./extract_orders.py
    python3 ./format_data.py
    cd ../..
    ```

## Generating the Iterative Updates
From the home directory, run `bazel build -c opt ...` to build all of the programs. 
The commands needed to generate the model updates for iterative solves are as follows:
1. Minimum Spanning Tree:
    ```shell script
    export GRAPHDIR="./tools/graphs/graph_protos/"
    export MST_OUTDIR="./protos/mst/"
   ./bazel-bin/math_opt_benchmark/mst/mst_main --graph_dir=$GRAPHDIR --out_dir=$MST_OUTDIR | egrep -v "Academic license"
    ```
2. Uncapacitated Facility Location: <br> ORLIB contains the solutions to each instance, so we check correctness by comparing
our output to theirs.
    ```shell script
    export UFL_OUTDIR="./math_opt_benchmark/facility/protos/"
    for file in `cat ./tools/facility/filepaths.txt`; do
      ./bazel-bin/math_opt_benchmark/facility/ufl_main --filename=$file --out_dir=$UFL_OUTDIR | egrep -v "Academic license" | diff $file.opt -
    done
    ```
3. Split Shipments with Substitutions
   ```shell script
   export CARTDIR="./tools/retail/dataset/"
   export SUB_OUTDIR="./protos/subs/"
   ./bazel-bin/math_opt_benchmark/retail/split_and_sub/split_and_sub_main --data_dir=$CARTDIR --solver_type=gurobi --solve_directly  | egrep -v "Academic license"
   ```
4. Split Shipments can be updated by a copy/paste/delete from substitutions once everything is working

### Running the Benchmarks
The script `./tools/benchmark.sh` will compare solver performance on all of the problems above, assuming default locations.
```shell script
bash ./tools/benchmark.sh
```
Every file in the `--instance_dir` directory will be run in the benchmark, so the only files in the directory should
be text protos of [BenchmarkInstance](math_opt_benchmark/proto/model.proto).

To benchmark individual models, add the `--is_single_file` flag
```shell script
./bazel-bin/math_opt_benchmark/benchmarker/benchmark_main --instance_dir=./protos/subs/11.txt --is_mip --is_single_file
```

### Directories
[math_opt_benchmark/benchmarker](math_opt_benchmark/benchmarker) contains the main program which reads the BenchmarkInstance
protos and compares solver performances. The protos are generated from the programs in the other
directories:
1. [math_opt_benchmark/facility](math_opt_benchmark/facility) contains the files to solve UFL
    * [knapsack.cc](math_opt_benchmark/facility/knapsack.cc) solves the worker dual
2. [math_opt_benchmark/mst](math_opt_benchmark/mst) contains the files to solve MST
    * [graph](math_opt_benchmark/mst/graph) contains the max-flow separation oracle and
    various graph algorithms for verifying the correctness of the MST.
3. [math_opt_benchmark/retail](math_opt_benchmark/retail) contains the shipment models
    * Not updated yet: [split](math_opt_benchmark/retail/split) minimizes split shipments
    * [split_and_sub](math_opt_benchmark/retail/split_and_sub) minimizes split shipments with personalized substitutions
4. [math_opt_benchmark/proto](math_opt_benchmark/proto) contains the protos used to store 
[graphs](math_opt_benchmark/proto/graph.proto), [Instacart orders](math_opt_benchmark/proto/dataset.proto), and the [BenchmarkInstance](math_opt_benchmark/proto/model.proto)