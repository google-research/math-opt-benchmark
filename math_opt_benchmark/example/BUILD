licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "example",
    srcs = ["example.cc"],
    hdrs = ["example.h"],
    deps = [
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_test(
    name = "example_test",
    srcs = ["example_test.cc"],
    deps = [
        ":example",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_binary(
    name = "example_main",
    srcs = ["example_main.cc"],
    deps = [
        ":example",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
