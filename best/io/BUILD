package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ioerr"
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
