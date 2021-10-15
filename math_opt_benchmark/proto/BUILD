licenses(["notice"])
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

package(
    default_visibility = [
        "//visibility:public",
    ],
)

proto_library(
    name = "model_proto",
    srcs = ["model.proto"],
    deps = [
        "@com_google_ortools//ortools/math_opt:model_proto",
        "@com_google_ortools//ortools/math_opt:model_update_proto",
    ],
)

proto_library(
    name = "dataset_proto",
    srcs = ["dataset.proto"],
    deps = [],
)

cc_proto_library(
    name = "model_cc_proto",
    deps = [
        ":model_proto",
    ],
)

cc_proto_library(
    name = "dataset_cc_proto",
    deps = [
        ":dataset_proto",
    ],
)

cc_binary(
    name = "test_main",
    srcs = ["test.cc"],
    deps = [
        ":model_cc_proto",
        ":dataset_cc_proto",
        "@com_google_absl//absl/random",
    ],
)
