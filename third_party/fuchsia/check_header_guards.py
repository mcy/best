# !/usr/bin/env fuchsia-vendored-python
# Copyright 2016 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Script to check C and C++ file header guards.

This script accepts a list of file or directory arguments. If a given
path is a file, it runs the checker on it. If the path is a directory,
it runs the checker on all files in that directory.

In addition, this script checks for potential header guard
collisions. This is useful since we munge / to _, and so
    lib/abc/xyz/xyz.h
and
    lib/abc_xyz/xyz.h
both want to use LIB_ABC_XYZ_XYZ_H_ as a header guard.

"""

import argparse
import collections
import fileinput
import os.path
import re
import string
import sys

FUCHSIA_ROOT = os.path.dirname(  # $root
    os.path.dirname(  # scripts
        os.path.dirname(os.path.realpath(os.path.abspath(__file__)))  # style
    )
)

SYSROOT_PREFIXES = ["ZIRCON_THIRD_PARTY_ULIB_MUSL_INCLUDE"]
sysroot_prefix = re.compile("^(" + "|".join(SYSROOT_PREFIXES) + ")_")

PUBLIC_PREFIXES = [
    "ZIRCON_SYSTEM_PUBLIC",
    "ZIRCON_SYSTEM_ULIB_.*_INCLUDE",
    "SDK(_.*_INCLUDE)?",
]
public_prefix = re.compile("^(" + "|".join(PUBLIC_PREFIXES) + ")_")

all_header_guards = collections.defaultdict(list)

pragma_once = re.compile("^#pragma once$")
disallowed_header_characters = re.compile("[^a-zA-Z0-9_]")


def adjust_for_location(header_guard):
    """Remove internal location prefix from public headers if applicable."""
    # Remove public prefixes
    header_guard = public_prefix.sub("", header_guard, 1)
    # Replace sysroot prefixes
    header_guard = sysroot_prefix.sub("SYSROOT_", header_guard, 1)

    return header_guard


def header_guard_from_path(path):
    """Compute the header guard from the path"""
    assert path.startswith(FUCHSIA_ROOT)
    relative_path = path[len(FUCHSIA_ROOT) :].strip("/")
    upper_path = relative_path.upper()
    header_guard = re.sub(disallowed_header_characters, "_", upper_path) + "_"
    header_guard = adjust_for_location(header_guard)
    return header_guard


def check_file(path, fix_guards=False):
    """Check whether the file has a correct header guard.

    A header guard can either be a #pragma once, or else a matching set of
        #ifndef PATH_TO_FILE_
        #define PATH_TO_FILE_
        ...
        #endif  // PATH_TO_FILE_
    preprocessor directives, where both '.' and '/' in the path are
    mapped to '_', and a trailing '_' is appended.

    In either the #pragma once case or the header guard case, it is
    assumed that there is no trailing or leading whitespace.

    """

    # Only check .h files
    if path[-2:] != ".h":
        return True

    header_guard = header_guard_from_path(path)
    all_header_guards[header_guard].append(path)

    ifndef = re.compile("^#ifndef %s$" % header_guard)
    define = re.compile("^#define %s$" % header_guard)
    endif = re.compile("^#endif +// *%s$" % header_guard)

    found_pragma_once = False
    found_ifndef = False
    found_define = False
    found_endif = False

    with open(path, "r") as f:
        for line in f.readlines():
            match = pragma_once.match(line)
            if match:
                if found_pragma_once:
                    print("%s contains multiple #pragma once" % path)
                    return False
                found_pragma_once = True

            match = ifndef.match(line)
            if match:
                if found_ifndef:
                    print("%s contains multiple ifndef header guards" % path)
                    return False
                found_ifndef = True

            match = define.match(line)
            if match:
                if found_define:
                    print("%s contains multiple define header guards" % path)
                    return False
                found_define = True

            match = endif.match(line)
            if match:
                if found_endif:
                    print("%s contains multiple endif header guards" % path)
                    return False
                found_endif = True

    if found_pragma_once:
        if found_ifndef or found_define or found_endif:
            print("%s contains both #pragma once and header guards" % path)
            return False
        if not fix_guards:
            return True

    if found_ifndef and found_define and found_endif:
        return True

    if not found_ifndef:
        print("%s did not contain ifndef part of its header guard" % path)
    elif not found_define:
        print("%s did not contain define part of its header guard" % path)
    elif not found_endif:
        print("%s did not contain endif part of its header guard" % path)
    elif fix_guards:
        if found_pragma_once:
            print("%s contained #pragma once instead of a header guard" % path)
        else:
            print(
                "%s did not contain a header guard or the header guard did "
                "not match the file path" % path
            )
    else:
        print(
            "%s contained neither a proper header guard nor #pragma once" % path
        )

    header_guards_fixed = False
    if fix_guards:
        header_guards_fixed = fix_header_guard(path, header_guard)

    if not header_guards_fixed:
        print(
            "Allowable header guard values are %s"
            % list(all_header_guards.keys())
        )

    return False


def fix_header_guard(path, header_guard):
    """Attempt to fix the header guard in the given file."""
    ifndef = re.compile("^#ifndef [^\s]+_H_$")
    define = re.compile("^#define [^\s]+_H_$")
    endif = re.compile("^#endif +// *[^\s]+_H_$")
    fixed_ifndef = False
    fixed_define = False
    fixed_endif = False
    fixed_pragma_once = False

    for line in fileinput.input(path, inplace=1):
        (new_line, changes) = re.subn(ifndef, "#ifndef %s" % header_guard, line)
        if changes:
            fixed_ifndef = True
            sys.stdout.write(new_line)
            continue
        (new_line, changes) = re.subn(define, "#define %s" % header_guard, line)
        if changes:
            fixed_define = True
            sys.stdout.write(new_line)
            continue
        (new_line, changes) = re.subn(
            endif, "#endif  // %s" % header_guard, line
        )
        if changes:
            fixed_endif = True
            sys.stdout.write(new_line)
            continue
        if pragma_once.match(line):
            fixed_pragma_once = True
            sys.stdout.write("#ifndef %s\n" % header_guard)
            sys.stdout.write("#define %s\n" % header_guard)
            continue
        sys.stdout.write(line)

    if fixed_pragma_once:
        with open(path, "a") as file:
            file.write("\n")
            file.write("#endif  // %s\n" % header_guard)

    if (fixed_ifndef and fixed_define and fixed_endif) or fixed_pragma_once:
        print("Fixed!")
        return True

    print("Not fixed...")
    return False


def check_dir(p, fix_guards=False):
    """Walk recursively over a directory checking .h files"""

    def prune(d):
        if d[0] == "." or d == "third_party":
            return True
        return False

    for root, dirs, paths in os.walk(p):
        # Prune dot directories like .git
        [dirs.remove(d) for d in list(dirs) if prune(d)]
        for path in paths:
            check_file(os.path.join(root, path), fix_guards=fix_guards)


def check_collisions():
    for header_guard, paths in all_header_guards.items():
        if len(paths) == 1:
            continue
        print("Multiple files could use %s as a header guard:" % header_guard)
        for path in paths:
            print("    %s" % path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--fix", help="Correct wrong header guards", action="store_true"
    )
    (arg_results, other_args) = parser.parse_known_args()
    fix_guards = arg_results.fix
    for p in other_args:
        p = os.path.realpath(os.path.abspath(p))
        if os.path.isdir(p):
            check_dir(p, fix_guards=fix_guards)
        else:
            check_file(p, fix_guards=fix_guards)
    check_collisions()


if __name__ == "__main__":
    sys.exit(main())