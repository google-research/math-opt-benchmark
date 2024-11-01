load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("//third_party/protobuf/bazel:proto_library.bzl", "proto_library")

licenses(["notice"])

package(
    default_applicable_licenses = ["//third_party/math_opt_benchmark:license"],
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

cc_proto_library(
    name = "model_cc_proto",
    deps = [
        ":model_proto",
    ],
)
