package(default_visibility = ["//visibility:public"])

cc_library(
  name = "encoding",
  hdrs = ["encoding.h"],
  deps = [
    "//best/container:result",
    "//best/memory:span",
  ]
)


cc_library(
  name = "rune",
  hdrs = ["rune.h"],
  deps = [
    ":encoding",
    "//best/log/internal:crash",
  ]
)

cc_test(
  name = "rune_test",
  srcs = ["rune_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":rune",
    "//best/test",
  ]
)

cc_library(
  name = "ascii",
  hdrs = ["ascii.h"],
  deps = [
    ":rune",
    "//best/base:guard",
    "//best/container:option",
    "//best/memory:span",
  ]
)

cc_library(
  name = "utf",
  hdrs = [
    "utf8.h",
    "utf16.h",
    "utf32.h",
    "internal/utf.h",
  ],
  deps = [
    ":rune",
    "//best/base:guard",
    "//best/container:option",
    "//best/memory:span",
  ]
)

cc_test(
  name = "utf_test",
  srcs = ["utf_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":utf",
    "//best/test",
  ]
)

cc_library(
  name = "format",
  hdrs = [
    "format.h",
    "internal/format_impls.h",
    "internal/format_parser.h",
  ],
  srcs = ["format.cc"],
  deps = [
    ":rune",
    ":strbuf",
    "//best/base:guard",
    "//best/container:option",
    "//best/math:conv",
    "//best/memory:span",
    "//best/meta:reflect",
  ]
)

cc_test(
  name = "format_parser_test",
  srcs = ["internal/format_parser_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":format",
    "//best/test",
  ]
)

cc_test(
  name = "format_test",
  srcs = ["format_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":format",
    "//best/test",
  ]
)

cc_library(
  name = "str",
  hdrs = ["str.h"],
  deps = [
    ":rune",
    ":utf",
    "//best/memory:span",
  ]
)

cc_test(
  name = "str_test",
  srcs = ["str_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":str",
    "//best/test",
  ]
)

cc_library(
  name = "strbuf",
  hdrs = ["strbuf.h"],
  deps = [
    ":ascii",
    ":str",
    "//best/container:vec",
  ]
)

cc_test(
  name = "strbuf_test",
  srcs = ["strbuf_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":strbuf",
    "//best/test",
  ]
)