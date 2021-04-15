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
#        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/strings",
        "@com_google_ortools//ortools/linear_solver",
        "@com_google_ortools//ortools/math_opt",
    ],
)

cc_test(
    name = "split_and_sub_test",
    srcs = [],
    deps = [
        ":split_and_sub",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@com_google_ortools//ortools/linear_solver",
        "@com_google_ortools//ortools/math_opt",
    ],
)

cc_binary(
    name = "split_and_sub_main",
    srcs = ["split_and_sub_main.cc"],
    deps = [
        ":split_and_sub",
        "@com_google_absl//absl/random",
        "@com_google_ortools//ortools/linear_solver",
    ],
)
