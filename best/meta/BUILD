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
  name = "traits",
  hdrs = [
    "traits.h",
    "internal/traits.h",
  ],
)

cc_library(
  name = "taxonomy",
  hdrs = [
    "taxonomy.h",
    "internal/abominable.h",
    "internal/quals.h",
  ],
  deps = [
    ":traits",
    "//best/base:hint",
  ],
)

cc_library(
  name = "ops",
  hdrs = [
    "ops.h",
    "internal/ops.h",
  ],
  deps = [
    "//best/base:hint",
  ]
)

cc_library(
  name = "guard",
  hdrs = ["guard.h"],
  deps = [
    ":tlist",
  ]
)

cc_library(
  name = "init",
  hdrs = [
    "init.h",
    "internal/init.h",
  ],
  deps = [
    ":bit_enum",
    ":tags",
    ":taxonomy",
  ]
)

cc_test(
  name = "init_test",
  srcs = ["init_test.cc"],
  deps = [
    ":init",
    "//best/container:vec",
    "//best/test:fodder",
  ]
)

cc_library(
  name = "ebo",
  hdrs = ["ebo.h"],
  deps = [
    ":init",
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
    ":traits",
    "//best/base:fwd",
    "//best/base:ord",
    "//best/container:bounds",
    "//best/func:call",
  ]
)

cc_test(
  name = "tlist_test",
  srcs = ["tlist_test.cc"],
  deps = [
    ":tlist",
  ]
)