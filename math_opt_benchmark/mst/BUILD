licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "mst",
    srcs = ["mst.cc"],
    hdrs = ["mst.h"],
    deps = [
        "//math_opt_benchmark/mst/graph",
        "//math_opt_benchmark/mst/matrix",
        "//math_opt_benchmark/proto:model_cc_proto",
        "//math_opt_benchmark/proto:graph_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
    ],
)

cc_test(
    name = "mst_test",
    srcs = [],
    deps = [
        ":mst",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_binary(
    name = "mst_main",
    srcs = ["mst_main.cc"],
    deps = [
        ":mst",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
