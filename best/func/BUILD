package(default_visibility = ["//visibility:public"])

cc_library(
  name = "arrow",
  hdrs = ["arrow.h"],
  deps = [
  ]
)

cc_library(
  name = "call",
  hdrs = [
    "call.h",
    "internal/call.h"
  ],
  deps = [
    "//best/base:hint",
    "//best/meta:init",
    "//best/meta/traits:empty",
    "//best/meta/traits:objects",
    "//best/meta/traits:funcs",
    "//best/meta/traits:types",
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
    "//best/meta/traits:funcs",
    "//best/meta/traits:ptrs",
    "//best/meta/traits:refs",
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