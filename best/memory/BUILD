package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ptr",
  hdrs = [
    "ptr.h",
    "internal/ptr.h",
  ],
  deps = [
    "//best/base:access",
    "//best/base:hint",
    "//best/base:niche",
    "//best/base:ord",
    "//best/meta:init",
    "//best/meta:tlist",
    "//best/meta/traits:types",
  ],
)

cc_library(
  name = "dyn",
  hdrs = [
    "dyn.h",
    "internal/dyn.h",  
  ],
  deps = [
    ":layout",
    "//best/func:fnref",
    "//best/base:access",
    "//best/meta/traits:types",
  ],
)

cc_test(
  name = "dyn_test",
  srcs = ["dyn_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":dyn",
    "//best/container:box",
    "//best/test",
  ],
)

cc_test(
  name = "ptr_test",
  srcs = ["ptr_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":ptr",
    "//best/test",
  ],
)

cc_library(
  name = "span",
  hdrs = [
    "span.h",
    "span_sort.h",
    "internal/bytes.h",
  ],
  deps = [
    "//best/container:option",
    "//best/container:result",
    "//best/iter",
  ],
)

cc_test(
  name = "span_test",
  srcs = ["span_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":span",
    "//best/test",
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
    "//best/meta/traits:arrays",
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
    ":ptr",
    "//best/meta:init",
  ]
)