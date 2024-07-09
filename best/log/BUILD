package(default_visibility = ["//visibility:public"])

cc_library(
  name = "location",
  hdrs = ["location.h"],
  deps = [
    "//best/base:port",
    "//best/base:fwd",
    "//best/meta:taxonomy",
    "//best/func:fnref",
  ],
)

cc_test(
  name = "location_test",
  srcs = ["location_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":location",
    "//best/test",
  ]
)

cc_library(
  name = "wtf",
  hdrs = ["wtf.h"],
  deps = [
    "//best/text:format",
  ],
)