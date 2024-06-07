package(default_visibility = ["//visibility:public"])

cc_library(
  name = "encoding",
  hdrs = ["encoding.h"],
  deps = [
    "//best/container:option",
    "//best/container:span",
  ]
)

cc_library(
  name = "rune",
  hdrs = ["rune.h"],
  deps = [
    ":encoding",
    ":utf_internal",
    "//best/log/internal:crash",
    "//best/container:span",
  ]
)

cc_library(
  name = "utf",
  hdrs = [
    "utf.h",
  ],
  deps = [
    ":encoding",
    ":rune",
    ":utf_internal",
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
  name = "utf_internal",
  hdrs = ["internal/utf.h"],
  deps = [
    "//best/container:option",
    "//best/container:span",
  ]
)