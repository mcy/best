#!/bin/bash

# Formats the repository.


usage() {
  echo "usage: $0 [OPTIONS]"
  echo "Format the repository."
  echo
  echo "  -h, --help      Show this message."
  echo "  -i, --in-place  Reformat in-place."
}

in_place=0
while [[ $# -gt 0 ]]; do
  case $1 in
    -h | --help)
      usage
      exit 0
      ;;
    -i | --in-place)
      in_place=1
      ;;
    *)
      echo "invalid option: $1" >&2
      usage
      exit 1
      ;;
  esac
  shift
done

# Find clang-format.
set -e
./bazel build --color=yes --curses=no @llvm//:clang-format
CLANG_FORMAT=$(./bazel cquery \
    --color=yes \
    --curses=no \
    --output=starlark \
    --starlark:expr='target.files_to_run.executable.path' \
     @llvm//:clang-format)
echo
set +e

LICENSE='/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

'
LICENSE_LINES=$(wc -l <<< "$LICENSE")

function license() {
  local f=$1
  if [[ $LICENSE == $(head -n $LICENSE_LINES "$f") ]]; then
    return 0
  fi
  
  if [[ $in_place != 0 ]]; then
    # For all C++ the first interesting line is a #directive.
    local first=$(grep -oahnm 1 "^#" "$f" | grep -oP '\d+')
    local rest=$(tail -n +$first "$f")
    echo "$LICENSE$rest" > $f
  else
    echo "$f has incorrect license header"
    return 1
  fi
}

bad=0
total=0
for file in $(find . -name '*.cc' -or -name '*.h'); do
  license "$file"
  bad=$((bad + $?))

  if [[ $in_place != 0 ]]; then
    $CLANG_FORMAT -i "$file"
  else
    $CLANG_FORMAT "$file" | \
      git --no-pager \
        diff --color=always --no-index --exit-code \
        "$file" - 
    bad=$((bad + $?))
    if [[ $? != 0 ]]; then
      echo
    fi
  fi
  total=$((total + 1))
done

if [[ $in_place != 0 ]]; then
  ./bazel run --color=yes --curses=no //third_party/fuchsia:format_guards -- --fix
else
  ./bazel run --color=yes --curses=no //third_party/fuchsia:format_guards
  bad=$((bad + $?))
fi

if [[ $bad -gt 0 ]]; then
  exit 1
fi