package(default_visibility = ["//visibility:public"])

cc_library(
  name = "rune",
  hdrs = ["rune.h"],
  deps = [
    "//best/container:span",
    "//best/container:result",
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
  name = "utf",
  hdrs = [
    "utf.h",
    "internal/utf.h",
  ],
  deps = [
    ":rune",
    "//best/container:option",
    "//best/container:span",
    "//best/meta:guard",
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
  name = "str",
  hdrs = ["str.h"],
  deps = [
    ":rune",
    ":utf",
    "//best/container:span",
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