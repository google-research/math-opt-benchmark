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
    name = "example",
    srcs = ["example.cc"],
    hdrs = ["example.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/base",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
    ],
)

cc_test(
    name = "example_test",
    srcs = ["example_test.cc"],
    deps = [
        ":example",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
    ],
)

cc_binary(
    name = "example_main",
    srcs = ["example_main.cc"],
    deps = [
        ":example",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/base",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_ortools//ortools/math_opt/solvers:gscip_solver",
    ],
)
