package(default_visibility = ["//visibility:public"])


cc_library(
  name = "bit",
  hdrs = ["bit.h"],
  deps = [
    ":int",
    ":overflow",
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

cc_library(
  name = "conv",
  hdrs = ["conv.h"],
  deps = [
    ":int",
    ":overflow",
    "//best/container:result",
    "//best/meta:guard",
    "//best/text:rune",
  ]
)

cc_test(
  name = "conv_test",
  srcs = ["conv_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":conv",
    "//best/test",
  ]
)

cc_library(
  name = "int",
  hdrs = [
    "int.h",
    "internal/common_int.h",
    ],
  deps = [
    "//best/base:fwd",
    "//best/meta:taxonomy",
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
  hdrs = ["overflow.h"],
  deps = [
    ":int",
    "//best/base:ord",
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