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
        "//math_opt_benchmark//proto:model_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver:linear_solver_glop",
        "@com_google_ortools//ortools/linear_solver:linear_solver_scip",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
    ],
)

cc_binary(
    name = "ufl_main",
    srcs = ["ufl_main.cc"],
    deps = [
        ":ufl",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver:linear_solver_glop",
        "@com_google_ortools//ortools/linear_solver:linear_solver_scip",
    ],
)
