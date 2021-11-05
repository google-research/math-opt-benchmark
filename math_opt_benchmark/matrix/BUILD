licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "matrix",
    srcs = [],
    hdrs = ["matrix.h"],
    deps = [
        "@com_github_glog_glog//:glog",
    ],
)

cc_test(
    name = "matrix_test",
    srcs = ["matrix_test.cc"],
    deps = [
        ":matrix",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)