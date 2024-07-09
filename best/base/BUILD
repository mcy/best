package(default_visibility = ["//visibility:public"])

cc_library(
  name = "fwd",
  hdrs = ["fwd.h"],
  deps = [
    ":tags",
    "//best/meta:taxonomy",
  ]
)

cc_library(
  name = "port",
  hdrs = ["port.h"],
)

cc_library(
  name = "tags",
  hdrs = ["tags.h"],
)

cc_library(
  name = "hint",
  hdrs = ["hint.h"],
  deps = [":port"],
)

cc_library(
  name = "unsafe",
  hdrs = ["unsafe.h"],
  deps = [":port"],
)

cc_library(
  name = "guard",
  hdrs = ["guard.h"],
  deps = [
    ":tags",
    "//best/meta:taxonomy",
  ]
)

cc_library(
  name = "ord",
  hdrs = [
    "ord.h",
    "internal/ord.h",
  ],
  deps = [
    ":port",
    "//best/func:call",
    "//best/meta:init",
    "//best/meta:taxonomy",
  ],
)

cc_test(
  name = "ord_test",
  srcs = ["ord_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":ord",
    "//best/test",
  ],
)