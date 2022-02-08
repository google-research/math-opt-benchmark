licenses(["notice"])

package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_binary(
    name = "cutting_stock_main",
    srcs = ["cutting_stock_main.cc"],
    deps = [
        "@com_google_ortools//ortools/base",
        "@com_google_ortools//ortools/math_opt/cpp:math_opt",
        "@com_google_ortools//ortools/math_opt/solvers:cp_sat_solver",
        "@com_google_ortools//ortools/math_opt/solvers:glop_solver",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "//math_opt_benchmark/proto:model_cc_proto",
    ],
)