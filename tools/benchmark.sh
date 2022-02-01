MST_OUTDIR="./protos/mst/"
UFL_OUTDIR="./math_opt_benchmark/facility/protos/"
# SUB_OUTDIR="./protos/subs/"

outdirs=($MST_OUTDIR $UFL_OUTDIR)
requires_mip=(1 1)

for i in "${!outdirs[@]}"
do
  if $requires_mip[i]
  then
    ./bazel-bin/math_opt_benchmark/benchmarker/benchmark_main --instance_dir=$outdirs[i] --is_mip
  else
    ./bazel-bin/math_opt_benchmark/benchmarker/benchmark_main --instance_dir=$outdirs[i]
  fi
done