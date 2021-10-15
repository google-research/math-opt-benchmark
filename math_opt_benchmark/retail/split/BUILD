licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "split",
    srcs = ["split.cc"],
    hdrs = ["split.h"],
    deps = [
#        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_test(
    name = "split_test",
    srcs = [],
    deps = [
        ":split",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
    ],
)

cc_binary(
    name = "split_main",
    srcs = ["split_main.cc"],
    deps = [
        ":split",
#        "@com_github_gflags_gflags//:gflags",
#        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
