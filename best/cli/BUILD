package(default_visibility = ["//visibility:public"])

cc_library(
  name = "app",
  hdrs = ["app.h"],
  srcs = ["app.cc"],
  deps = [
    ":cli",
    ":parser",
    "//best/log:wtf",
    "//best/text:strbuf",
    "//best/text:format",
  ]
)

cc_library(
  name = "cli",
  hdrs = ["cli.h"],
  srcs = ["cli.cc"],
  deps = [
    "//best/container:result",
    "//best/meta:init",
    "//best/text:format",
    "//best/text:str",
  ]
)

cc_library(
  name = "parser",
  hdrs = ["parser.h"],
  srcs = ["parser.cc"],
  deps = [
    ":cli",
    "//best/container:result",
    "//best/log:wtf",
    "//best/math:conv",
    "//best/memory:allocator",
    "//best/meta:init",
    "//best/meta:reflect",
    "//best/meta:taxonomy",
    "//best/text:format",
    "//best/text:str",
    "//best/text:strbuf",
    "//best/text:utf",
  ]
)

cc_test(
  name = "cli_test",
  srcs = ["cli_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":cli",
    ":parser",
    ":toy_flags",
    "//best/test",
  ]
)

cc_library(
  name = "toy_flags",
  hdrs = ["toy_flags.h"],
  deps = [
    ":cli",
  ],
  visibility = ["//visibility:private"],
)

cc_binary(
  name = "toy",
  srcs = ["toy.cc"],
  deps = [
    ":app",
    ":toy_flags",
  ]
)