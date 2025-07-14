load("//third_party/bazel_rules/rules_cc/cc:cc_binary.bzl", "cc_binary")
load("//third_party/bazel_rules/rules_cc/cc:cc_library.bzl", "cc_library")
load("//third_party/bazel_rules/rules_cc/cc:cc_test.bzl", "cc_test")

licenses(["notice"])

package(
    default_applicable_licenses = ["//third_party/math_opt_benchmark:license"],
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "ufl",
    srcs = ["ufl.cc"],
    hdrs = ["ufl.h"],
    tags = ["not_build:arm"],
    deps = [
        "//math_opt_benchmark/proto:model_cc_proto",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/base",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gurobi_solver",
    ],
)

cc_binary(
    name = "ufl_main",
    srcs = ["ufl_main.cc"],
    tags = ["not_build:arm"],
    deps = [
        ":ufl",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/base",
    ],
)

cc_test(
    name = "ufl_test",
    srcs = ["ufl_test.cc"],
    tags = ["not_build:arm"],
    deps = [
        ":ufl",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
    ],
)
