licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "split_and_sub",
    srcs = ["split_and_sub.cc"],
    hdrs = ["split_and_sub.h"],
    deps = [
        "//math_opt_benchmark/proto:model_cc_proto",
        "//math_opt_benchmark/proto:dataset_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
    ],
)

cc_test(
    name = "split_and_sub_test",
    srcs = [],
    deps = [
        ":split_and_sub",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "split_and_sub_main",
    srcs = ["split_and_sub_main.cc"],
    deps = [
        ":split_and_sub",
        "@com_google_absl//absl/random",
    ],
)
