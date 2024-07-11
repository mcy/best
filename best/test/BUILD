package(default_visibility = ["//visibility:public"])

cc_library(
  name = "fodder",
  hdrs = ["fodder.h"],
  srcs = ["fodder.cc"],
  testonly = True,
  deps = [
    ":test",
    "//best/text:str",
    "//best/log:location",
  ],
)

cc_library(
  name = "test_nomain",
  hdrs = ["test.h"],
  srcs = ["test.cc"],
  testonly = True,
  deps = [
    "//best/cli:app",
    "//best/container:vec",
    "//best/text:str",
    "//best/text:format",
    "//best/log:location",
  ],
)

cc_library(
  name = "test",
  srcs = ["test_main.cc"],
  testonly = True,
  deps = [
    ":test_nomain",
    "//best/cli:app",
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