package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ptr",
  hdrs = [
    "ptr.h",
    "internal/ptr.h",
  ],
  deps = [
    "//best/base:hint",
    "//best/base:niche",
    "//best/base:ord",
    "//best/meta:init",
    "//best/meta:taxonomy",
    "//best/meta:tlist",
    "//best/meta:traits",
  ],
)

cc_library(
  name = "layout",
  hdrs = [
    "layout.h",
    "internal/layout.h"
  ],
  deps = [
    "//best/base:unsafe",
    "//best/math:overflow",
    "//best/meta:ops",
    "//best/meta:tlist",
  ]
)

cc_test(
  name = "layout_test",
  srcs = ["layout_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":layout",
    "//best/test",
  ],
)

cc_library(
  name = "allocator",
  hdrs = ["allocator.h"],
  srcs = ["allocator.cc"],
  deps = [
    ":layout",
    "//best/meta:init",
  ]
)

cc_library(
  name = "bytes",
)