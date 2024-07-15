package(default_visibility = ["//visibility:public"])

cc_library(
  name = "arrow",
  hdrs = ["arrow.h"],
  deps = [
    "//best/meta:taxonomy",
  ]
)

cc_library(
  name = "call",
  hdrs = [
    "call.h",
    "internal/call.h"
  ],
  deps = [
    "//best/base:port",
    "//best/meta:empty",
    "//best/meta:init",
  ],
)

cc_library(
  name = "fnref",
  hdrs = [
    "fnref.h",
    "internal/fnref.h",
  ],
  deps = [
    ":call",
  ]
)

cc_test(
  name = "fnref_test",
  srcs = ["fnref_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":fnref",
    "//best/test",
  ],
)

cc_library(
  name = "tap",
  hdrs = ["tap.h"],
  deps = [
    ":call",
  ],
)

cc_test(
  name = "tap_test",
  srcs = ["tap_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":tap",
    "//best/test",
  ],
)