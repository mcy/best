package(default_visibility = ["//visibility:public"])

cc_library(
  name = "arrays",
  hdrs = ["arrays.h"],
  deps = [
    "//best/base:fwd",
  ]
)

cc_library(
  name = "classes",
  hdrs = ["classes.h"],
)

cc_library(
  name = "empty",
  hdrs = ["empty.h"],
  deps = [
    ":types",
  ]
)

cc_library(
  name = "enums",
  hdrs = ["enums.h"],
)

cc_library(
  name = "funcs",
  hdrs = [
    "funcs.h",
    "internal/funcs.h",
  ],
)

cc_library(
  name = "objects",
  hdrs = ["objects.h"],
)

cc_library(
  name = "ptrs",
  hdrs = ["ptrs.h"],
  deps = [
    ":funcs",
  ]
)

cc_library(
  name = "quals",
  hdrs = [
    "quals.h",
    "internal/quals.h",
  ],
)

cc_library(
  name = "refs",
  hdrs = ["refs.h"],
  deps = [
    ":quals",
  ],
)

cc_library(
  name = "types",
  hdrs = [
    "types.h",
    "internal/types.h",
  ],
)
