package(default_visibility = ["//visibility:public"])

cc_library(
  name = "fodder",
  hdrs = ["fodder.h"],
  srcs = ["fodder.cc"],
  testonly = True,
  deps = [
    ":test",
    "//best/strings:str",
    "//best/log:location",
  ],
)

cc_library(
  name = "test",
  hdrs = ["test.h"],
  srcs = ["test.cc"],
  testonly = True,
  deps = [
    "//best/strings:str",
    "//best/log:location",
  ],
)

cc_test(
  name = "test_test",
  srcs = ["test_test.cc"],
  linkopts = ["-rdynamic"],
  deps = [
    ":test",
  ]
)