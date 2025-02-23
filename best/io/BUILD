package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ioerr",
  hdrs = ["ioerr.h"],
  srcs = [
    "errnos.inc",
    "ioerr.cc",
  ],
  deps = [
    "//best/log:wtf",
    "//best/text:str",
  ],
)

cc_library(
  name = "path",
  hdrs = ["path.h"],
  srcs = ["path.cc"],
  deps = [
    "//best/func:defer",
    "//best/container:vec",
    "//best/iter",
    "//best/text:str",
    "//best/text:utf",
  ],
)

cc_test(
  name = "path_test",
  srcs = ["path_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":path",
    "//best/test",
  ],
)
