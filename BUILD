load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_clangd",
    targets = "//...",
)

py_binary(
    name = "format_guards",
    srcs = [
      "format_guards.py",
      "//third_party/fuchsia:check_header_guards.py",
    ],
    args = ["."]
)