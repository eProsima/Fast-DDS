workspace(name = "Fast-DDS")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
  strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
)

http_archive(
    name = "rules_foreign_cc",
    sha256 = "6041f1374ff32ba711564374ad8e007aef77f71561a7ce784123b9b4b88614fc",
    strip_prefix = "rules_foreign_cc-0.8.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.8.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

# This sets up some common toolchains for building targets. For more details, please see
# https://bazelbuild.github.io/rules_foreign_cc/0.8.0/flatten.html#rules_foreign_cc_dependencies
rules_foreign_cc_dependencies()

_ALL_CONTENT = """\
filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)
"""

_ALL_HDRS = """\
filegroup(
    name = "all_hdrs",
    srcs = glob(["include/**/*.hpp","include/**/*.ipp"]),
    visibility = ["//visibility:public"],
)
"""

http_archive(
    name = "openssl",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "openssl-3.0.7",
    urls = [
        "https://www.openssl.org/source/openssl-3.0.7.tar.gz"
    ],
    sha256 = "83049d042a260e696f62406ac5c08bf706fd84383f945cf21bd61e9ed95c396e",
)

http_archive(
    name = "asio",
    build_file_content = _ALL_HDRS,
    strip_prefix = "asio-asio-1-24-0/asio",
    urls = [
        "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-24-0.zip"
    ],
    sha256 = "6bb8139ebc1c97a4364f6e517b9258ecc96345a3bfc4d110f931ac123dbdc824",
)

http_archive(
    name = "tinyxml2",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "tinyxml2-9.0.0",
    urls = [
        "https://github.com/leethomason/tinyxml2/archive/refs/tags/9.0.0.zip"
    ],
    sha256 = "6904137f290b0836792fd182c0d463cb66da085856b0e5803089b40bf379fdcf",
)

http_archive(
    name = "foonathan_memory",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "memory-0.7-2",
    urls = [
        "https://github.com/foonathan/memory/archive/refs/tags/v0.7-2.zip"
    ],
    sha256 = "96cc142166f01ee4dc8e6a06683c19b091417c141d34119ab9861af07b48480f",
)

http_archive(
    name = "fastcdr",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "Fast-CDR-1.0.25",
    urls = [
        "https://github.com/eProsima/Fast-CDR/archive/refs/tags/v1.0.25.zip"
    ],
    sha256 = "a203f64195ffd8168bb329617c1bbc3b00843c762e949084f012f415136b9403",
)

http_archive(
    name = "fastdds",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "Fast-DDS-2.8.1",
    urls = [
        "https://github.com/eProsima/Fast-DDS/archive/refs/tags/v2.8.1.zip"
    ],
    sha256 = "3d7332ae6bcd142182419e2883f89a14d808f308b2289f18984197ac85b1ab26",
)

http_archive(
    name = "fastddsgen",
    build_file_content = _ALL_CONTENT,
    strip_prefix = "Fast-DDS-Gen-2.2.0",
    urls = [
        "https://github.com/eProsima/Fast-DDS-Gen/archive/refs/tags/v2.2.0.zip"
    ],
    sha256 = "39e4efd263ee45c20877751468ce280963148754afbd49cb10e5650341be1129",
)
