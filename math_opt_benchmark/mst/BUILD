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
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
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
        "//math_opt_benchmark/mst/graph",
        "//math_opt_benchmark/mst/matrix",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
