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
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
        "//math_opt_benchmark/matrix",
    ],
)

cc_test(
    name = "ufl_test",
    srcs = [],
    deps = [
        ":ufl",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_binary(
    name = "ufl_main",
    srcs = ["ufl_main.cc"],
    deps = [
        ":ufl",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)