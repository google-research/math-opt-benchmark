licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "benchmarker",
    srcs = ["benchmarker.cc"],
    hdrs = ["benchmarker.h"],
    deps = [

    ],
)


cc_binary(
    name = "benchmark_main",
    srcs = ["benchmark_main.cc"],
    deps = [
        ":benchmarker",
        "//math_opt_benchmark/proto:model_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
    ],
)
