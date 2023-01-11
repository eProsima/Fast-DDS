load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake", "configure_make")

cc_library(
    name = "asio",
    hdrs = ["@asio//:all_hdrs"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"]
)

cmake(
    name = "tinyxml2",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "BUILD_SHARED_LIBS": "ON",
    },
    lib_source = "@tinyxml2//:all_srcs",
    out_shared_libs = select({
        "@bazel_tools//src/conditions:linux": [
            "libtinyxml2.so",
        ],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "libtinyxml2.so",
        ],
        "@bazel_tools//src/conditions:darwin": [
            "libtinyxml2.dylib",
        ],
        "//conditions:default": [],
    }),
)

cmake(
    name = "foonathan_memory",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
        "FOONATHAN_MEMORY_BUILD_TESTS": "OFF",
        "FOONATHAN_MEMORY_BUILD_EXAMPLES": "OFF",
        "FOONATHAN_MEMORY_BUILD_TOOLS": "OFF"
    },
    lib_source = "@foonathan_memory//:all_srcs",
    out_static_libs = ["libfoonathan_memory-0.7.2.a"],
)

# If encountering openssl build issues: https://github.com/bazelbuild/rules_foreign_cc/issues/337
configure_make(
    name = "openssl",
    lib_source = "@openssl//:all_srcs",
    # configure_options = ["--with-included-unistring"],
    configure_command = "Configure",
    out_include_dir = "include/openssl",
    out_lib_dir = select({
        "@bazel_tools//src/conditions:linux": "lib64",
        "@bazel_tools//src/conditions:linux_x86_64": "lib64",
        "@bazel_tools//src/conditions:darwin": "lib",
        "//conditions:default": "lib64",
    }),
    out_static_libs = ["libssl.a", "libcrypto.a"],
    out_shared_libs = select({
        "@bazel_tools//src/conditions:linux": [
            "libssl.so",
            "libcrypto.so"
        ],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "libssl.so",
            "libcrypto.so"
        ],
        "@bazel_tools//src/conditions:darwin": [
            "libssl.dylib",
            "libcrypto.dylib"
        ],
        "//conditions:default": [],
    }),
    env = {
        "AR": "",
    },
    visibility = ["//visibility:public"]
)

cmake(
    name = "fastcdr",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
    },
    lib_source = "@fastcdr//:all_srcs",
    out_shared_libs = select({
        "@bazel_tools//src/conditions:linux": [
            "libfastcdr.so"
        ],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "libfastcdr.so"
        ],
        "@bazel_tools//src/conditions:darwin": [
            "libfastcdr.dylib"
        ],
        "//conditions:default": [],
    }),
    deps = [
        "foonathan_memory"
    ]
)

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

cmake(
    name = "fastdds",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
    },
    lib_source = "//:all_srcs",
    out_binaries = ["fastdds"],
    out_shared_libs = select({
        "@bazel_tools//src/conditions:linux": [
            "libfastrtps.so"
        ],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "libfastrtps.so"
        ],
        "@bazel_tools//src/conditions:darwin": [
            "libfastrtps.dylib"
        ],
        "//conditions:default": [],
    }),
    deps = [
        "asio",
        "tinyxml2",
        "openssl",
        "foonathan_memory",
        "fastcdr"
    ]
)
