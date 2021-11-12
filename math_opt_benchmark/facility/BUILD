licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "ufl",
    srcs = ["ufl.cc"],
    hdrs = ["ufl.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "//math_opt_benchmark/proto:model_cc_proto",
    ],
)


cc_binary(
    name = "ufl_main",
    srcs = ["ufl_main.cc"],
    deps = [
        ":ufl",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
    ],
)