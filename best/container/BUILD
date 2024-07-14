package(default_visibility = ["//visibility:public"])

cc_library(
  name = "choice",
  hdrs = [
    "choice.h",
    "internal/choice.h",
  ],
  deps = [
    ":pun",
    "//best/base:port",
    "//best/base:tags",
    "//best/log/internal:crash",
  ],
)

cc_test(
  name = "choice_test",
  srcs = ["choice_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":choice",
    "//best/test",
    "//best/test:fodder",
  ],
)

cc_library(
  name = "object",
  hdrs = ["object.h"],
  deps = [
    "//best/base:ord",
    "//best/memory:ptr",
    "//best/meta:empty",
  ]
)

cc_test(
  name = "object_test",
  srcs = ["object_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":object",
    "//best/test",
  ]
)

cc_library(
  name = "option",
  hdrs = ["option.h"],
  deps = [
    ":choice",
    "//best/log/internal:crash",
  ],
)

cc_test(
  name = "option_test",
  srcs = ["option_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":option",
    ":result",
    "//best/test",
    "//best/test:fodder",
  ],
)

cc_library(
  name = "pun",
  hdrs = [
    "pun.h",
    "internal/pun.h",
  ],
  deps = [
    ":object",
    "//best/base:port",
    "//best/memory:layout",
    "//best/base:tags",
    "//best/meta:init",
  ],
)

cc_test(
  name = "pun_test",
  srcs = ["pun_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":pun",
    "//best/test",
    "//best/test:fodder",
  ],
)

cc_library(
  name = "result",
  hdrs = ["result.h"],
  deps = [
    ":choice",
    ":row",
    "//best/log/internal:crash",
  ],
)

cc_test(
  name = "result_test",
  srcs = ["result_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":option",
    ":result",
    "//best/test",
    "//best/test:fodder",
    "//best/text:strbuf",
  ],
)

cc_library(
  name = "row",
  hdrs = [
    "row.h",
    "internal/row.h",
  ],
  deps = [
    ":object",
    "//best/base:port",
    "//best/base:tags",
  ],
)

cc_test(
  name = "row_test",
  srcs = ["row_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":row",
    "//best/test",
    "//best/test:fodder",
  ],
)

cc_library(
  name = "span",
  hdrs = [
    "span.h",
    "span_sort.h",
    "internal/bytes.h",
  ],
  deps = [
    ":option",
    ":result",
    "//best/iter",
    "//best/memory:bytes",
  ],
)

cc_test(
  name = "span_test",
  srcs = ["span_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":span",
    "//best/test",
  ],
)

cc_library(
  name = "vec",
  hdrs = ["vec.h"],
  deps = [
    ":object",
    ":option",
    ":span",
    "//best/func:arrow",
    "//best/math:bit",
    "//best/math:overflow",
    "//best/memory:allocator",
    "//best/memory:bytes",
    "//best/memory:layout",
    "//best/meta:init",
    "//best/base:tags",
  ],
)

cc_test(
  name = "vec_test",
  srcs = ["vec_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":vec",
    "//best/test",
    "//best/test:fodder",
  ],
)


cc_library(
  name = "simple_option",
  hdrs = ["internal/simple_option.h"],
  visibility = ["//best:__subpackages__"],
  deps = [
    "//best/meta:taxonomy",
  ]
)
