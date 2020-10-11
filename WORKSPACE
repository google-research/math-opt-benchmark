load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# DO NOT CHECK IN
# Using this newer version of absl breaks the build, see cl description, look
# into it.
#
# Sept 2020
#http_archive(
#    name = "com_google_absl",
#    sha256 = "b3744a4f7a249d5eaf2309daad597631ce77ea62e0fc6abffbab4b4c3dc0fc08",
#    strip_prefix = "abseil-cpp-20200923",
#    urls = [
#        "https://mirror.bazel.build/github.com/abseil/abseil-cpp/archive/20200923.tar.gz",
#        "https://github.com/abseil/abseil-cpp/archive/20200923.tar.gz",
#    ],
#)

git_repository(
    name = "com_google_absl",
    commit = "c51510d",  # release 20200225.2
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

http_archive(
    name = "com_github_glog_glog",
    sha256 = "62efeb57ff70db9ea2129a16d0f908941e355d09d6d83c9f7b18557c0a7ab59e",
    strip_prefix = "glog-d516278b1cd33cd148e8989aec488b6049a4ca0b",
    urls = ["https://github.com/google/glog/archive/d516278b1cd33cd148e8989aec488b6049a4ca0b.zip"],
)

# August 2020
http_archive(
    name = "com_google_ortools",  # Apache 2.0
    sha256 = "d93a9502b18af51902abd130ff5f23768fcf47e266e6d1f34b3586387aa2de68",
    strip_prefix = "or-tools-7.8",
    urls = [
        "https://mirror.bazel.build/github.com/google/or-tools/archive/v7.8.tar.gz",
        "https://github.com/google/or-tools/archive/v7.8.tar.gz",
        "https://github.com/google/or-tools/archive/53189020e3f995715a935aab7355357ce658fb76.tar.gz",
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

# This is used by or-tools, but really or-tools should use absl flags.
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
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
    commit = "c9d2bd2",  # release v3.12.4
    remote = "https://github.com/protocolbuffers/protobuf.git",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

# Load common dependencies.
protobuf_deps()

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
