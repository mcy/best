package(default_visibility = ["//visibility:public"])

cc_library(
  name = "bounds",
  hdrs = ["bounds.h"],
  srcs = ["bounds.cc"],
  deps = [
    "//best/container:simple_option",
    "//best/log/internal:crash",
    "//best/math:overflow",
  ],
)

cc_test(
  name = "bounds_test",
  srcs = ["bounds_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":bounds",
    "//best/iter",
    "//best/test",
  ],
)

cc_library(
  name = "iter",
  hdrs = ["iter.h"],
  deps = [
    "//best/container:option",
    "//best/func:call",
  ],
)

cc_test(
  name = "iter_test",
  srcs = ["iter_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":iter",
    "//best/test",
  ],
)