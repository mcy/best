package(default_visibility = ["//visibility:public"])

cc_library(
  name = "int",
  hdrs = ["int.h"],
  deps = [
    "//best/base:fwd",
    "//best/meta:concepts",
  ],
)

cc_test(
  name = "int_test",
  srcs = ["int_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":int",
    "//best/test",
  ],
)

cc_library(
  name = "overflow",
  hdrs = [
    "overflow.h",
    "internal/common_int.h",
  ],
  deps = [
    ":int",
    "//best/base:fwd",
    "//best/meta:concepts",
  ],
)

cc_test(
  name = "overflow_test",
  srcs = ["overflow_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":overflow",
    "//best/test",
  ],
)

cc_library(
  name = "bit",
  hdrs = ["bit.h"],
  deps = [
    ":int",
    ":overflow",
    "//best/base:fwd",
    "//best/meta:concepts",
  ],
)

cc_test(
  name = "bit_test",
  srcs = ["bit_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":bit",
    "//best/test",
  ],
)