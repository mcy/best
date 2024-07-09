package(default_visibility = ["//visibility:public"])

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
  name = "init",
  hdrs = [
    "init.h",
    "internal/init.h",
  ],
  deps = [
    ":taxonomy",
    "//best/base:tags",
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
  name = "empty",
  hdrs = ["empty.h"],
  deps = [
    ":init",
  ]
)

cc_library(
  name = "names",
  hdrs = [
    "names.h",
    "internal/names.h",
  ],
  deps = [
    ":taxonomy",
    "//best/base:fwd",
    "//best/text:str",
  ],
)

cc_test(
  name = "names_test",
  srcs = ["names_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":names",
    "//best/test",
  ]
)


cc_library(
  name = "reflect",
  hdrs = [
    "reflect.h",
    "internal/reflect.h",
    "internal/reflect_bind.inc",
  ],
  deps = [
    ":names",
    ":taxonomy",
    "//best/base:fwd",
    "//best/container:row",
    "//best/func:tap",
    "//best/text:str",
  ],
)

cc_test(
  name = "reflect_test",
  srcs = ["reflect_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":reflect",
    "//best/test",
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
    ":traits",
    "//best/base:fwd",
    "//best/base:ord",
    "//best/func:call",
    "//best/iter:bounds",
  ]
)

cc_test(
  name = "tlist_test",
  srcs = ["tlist_test.cc"],
  deps = [
    ":tlist",
  ]
)