package(default_visibility = ["//visibility:public"])

cc_library(
  name = "hash",
  hdrs = [
    "hash.h",
    "internal/hash.h",
  ],
  deps = [
    "//best/base:fwd",
    "//best/base:ord",
    "//best/meta:init",
    "//best/meta/traits:types",
  ],
)

cc_library(
  name = "impls",
  hdrs = ["impls.h"],
  deps = [
    ":hash",
    "//best/memory:span",
    "//best/meta:reflect",
    "//best/text:str",
  ],
)


cc_library(
  name = "fxhash",
  hdrs = ["fxhash.h"],
  deps = [
    ":hash",
    "//best/math:bit",
    "//best/math:int",
    "//best/math:overflow",
    "//best/memory:span",
  ],
)