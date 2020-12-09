licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "graph",
    srcs = ["graph.cc"],
    hdrs = ["graph.h", "flow.h"],
    deps = [
        "//math_opt_benchmark/mst/matrix",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
        "@com_google_ortools//ortools/graph:max_flow",
    ],
)

cc_test(
    name = "graph_test",
    srcs = [],
    deps = [
        ":graph",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
