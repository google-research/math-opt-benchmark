load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_google_absl",
    commit = "997aaf3", # release 20210324.0
    remote = "https://github.com/abseil/abseil-cpp.git",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "774f5499dee0f9d2b583ce7fff62e575acce432b67a8396e86f3a87c90d2987e",
    strip_prefix = "googletest-604ba376c3a407c6a40e39fbd0d5055c545f9898",
    urls = [
        "https://mirror.bazel.build/github.com/google/googletest/archive/604ba376c3a407c6a40e39fbd0d5055c545f9898.tar.gz",
        "https://github.com/google/googletest/archive/604ba376c3a407c6a40e39fbd0d5055c545f9898.tar.gz",
    ],
)

#http_archive(
#    name = "com_github_glog_glog",
#    sha256 = "62efeb57ff70db9ea2129a16d0f908941e355d09d6d83c9f7b18557c0a7ab59e",
#    strip_prefix = "glog-d516278b1cd33cd148e8989aec488b6049a4ca0b",
#    urls = ["https://github.com/google/glog/archive/d516278b1cd33cd148e8989aec488b6049a4ca0b.zip"],
#)

# August 2020
#http_archive(
#    name = "com_google_ortools",  # Apache 2.0
#    sha256 = "beb9fe379977033151045d0815d26c628ad99d74d68b9f3b707578492723731e",
#    strip_prefix = "or-tools-8.1",
#    urls = [
#        "https://mirror.bazel.build/github.com/google/or-tools/archive/v8.1.tar.gz",
#        "https://github.com/google/or-tools/archive/v8.1.tar.gz",
#    ],
#)

# Math Opt
http_archive(
    name = "com_google_ortools",  # Apache 2.0
    sha256 = "89f80f54307e97aea9e6089e0d19fe75462fb28159b425dbd6183427583fce25",
    strip_prefix = "or-tools-master",
    urls = [
        "https://github.com/google/or-tools/archive/refs/heads/master.zip",
    ],
)

################################################################################
# or-tools dependencies
################################################################################

http_archive(
    name = "zlib",
    build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = [
        "https://mirror.bazel.build/zlib.net/zlib-1.2.11.tar.gz",
        "https://zlib.net/zlib-1.2.11.tar.gz",
    ],
)

git_repository(
    name = "bazel_skylib",
    commit = "e59b620",  # release 1.0.2
    remote = "https://github.com/bazelbuild/bazel-skylib.git",
)

# Python Rules
http_archive(
    name = "rules_python",
    sha256 = "b5668cde8bb6e3515057ef465a35ad712214962f0b3a314e551204266c7be90c",
    strip_prefix = "rules_python-0.0.2",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.0.2/rules_python-0.0.2.tar.gz",
)

# Protobuf
git_repository(
    name = "com_google_protobuf",
    commit = "878be35",  # release v3.15.7
    remote = "https://github.com/protocolbuffers/protobuf.git",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

# Load common dependencies.
protobuf_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "gtest",
    build_file = "@com_google_ortools//bazel:gtest.BUILD",
    strip_prefix = "googletest-release-1.8.0/googletest",
    url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
)

http_archive(
    name = "glpk",
    build_file = "@com_google_ortools//bazel:glpk.BUILD",
    sha256 = "4281e29b628864dfe48d393a7bedd781e5b475387c20d8b0158f329994721a10",
    url = "http://ftp.gnu.org/gnu/glpk/glpk-4.65.tar.gz",
)

http_archive(
    name = "bliss",
    build_file = "@com_google_ortools//bazel:bliss.BUILD",
    patches = ["@com_google_ortools//bazel:bliss-0.73.patch"],
    sha256 = "f57bf32804140cad58b1240b804e0dbd68f7e6bf67eba8e0c0fa3a62fd7f0f84",
    url = "http://www.tcs.hut.fi/Software/bliss/bliss-0.73.zip",
)

http_archive(
    name = "scip",
    build_file = "@com_google_ortools//bazel:scip.BUILD",
    patches = ["@com_google_ortools//bazel:scip.patch"],
    sha256 = "033bf240298d3a1c92e8ddb7b452190e0af15df2dad7d24d0572f10ae8eec5aa",
    url = "https://github.com/google/or-tools/releases/download/v7.7/scip-7.0.1.tgz",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# rules_cc defines rules for generating C++ code from Protocol Buffers.
http_archive(
    name = "rules_cc",
    sha256 = "35f2fb4ea0b3e61ad64a369de284e4fbbdcdba71836a5555abb5e194cf119509",
    strip_prefix = "rules_cc-624b5d59dfb45672d4239422fa1e3de1822ee110",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
        "https://github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
    ],
)

# rules_proto defines abstract rules for building Protocol Buffers.
http_archive(
    name = "rules_proto",
    sha256 = "2490dca4f249b8a9a3ab07bd1ba6eca085aaf8e45a734af92aad0c42d9dc7aaf",
    strip_prefix = "rules_proto-218ffa7dfa5408492dc86c01ee637614f8695c45",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/218ffa7dfa5408492dc86c01ee637614f8695c45.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/218ffa7dfa5408492dc86c01ee637614f8695c45.tar.gz",
    ],
)

load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies")
rules_cc_dependencies()

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
rules_proto_dependencies()
rules_proto_toolchains()