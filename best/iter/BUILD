package(default_visibility = ["//visibility:public"])

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