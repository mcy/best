package(default_visibility = ["//visibility:public"])

cc_library(
  name = "tags",
  hdrs = ["tags.h"],
)

cc_library(
  name = "bit_enum",
  hdrs = ["bit_enum.h"],
)

cc_library(
  name = "ops",
  hdrs = [
    "ops.h",
    "internal/ops.h",
  ],
)

cc_library(
  name = "init",
  hdrs = [
    "init.h",
    "internal/init.h",
  ],
  deps = [
    ":bit_enum",
    ":concepts",
    ":tags",
  ]
)

cc_test(
  name = "init_test",
  srcs = ["init_test.cc"],
  deps = [
    ":init",
    "//best/test:fodder",
  ]
)

cc_library(
  name = "tlist",
  hdrs = [
    "tlist.h",
    "internal/tlist.h",
  ],
  deps = [
    ":ops",
    "//best/base:fwd",
    "//best/container:bounds",
  ]
)

cc_test(
  name = "tlist_test",
  srcs = ["tlist_test.cc"],
  deps = [
    ":tlist",
  ]
)

cc_library(
  name = "concepts",
  hdrs = [
    "concepts.h",
    "internal/abominable.h",
    "internal/quals.h",
  ],
  deps = [
    ":bit_enum",
    ":tags",
    ":tlist",
  ]
)