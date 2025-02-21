package(default_visibility = ["//visibility:public"])

cc_library(
  name = "fwd",
  hdrs = ["fwd.h"],
  deps = [
    ":tags",
    "//best/meta/traits:objects",
    "//best/meta/traits:refs",
    "//best/meta/traits:types",
  ]
)

cc_library(
  name = "access",
  hdrs = ["access.h"],
  deps = [
    ":fwd",
  ],
)

cc_library(
  name = "port",
  hdrs = ["port.h"],
  deps = [":macro"],
)

cc_library(
  name = "macro",
  hdrs = [
    "internal/macro.h",
    "internal/macro.inc",
    "macro.h",
  ],
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
    "//best/meta/traits:refs",
  ]
)

cc_library(
  name = "niche",
  hdrs = ["niche.h"],
  deps = [
    ":ord",
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
    "//best/meta/traits:ptrs",
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