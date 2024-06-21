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

LICENSE_LINES=$(wc -l < ./LICENSE.inc)

function license() {
  local f=$1
  if [[ $in_place != 0 ]]; then
    # For all C++ the first interesting line is a #directive.
    local first=$(grep -oahnm 1 "^#" "$f" | grep -oP '\d+')
    local rest=$(tail -n +$first "$f")
    cat LICENSE.inc > $f
    echo >> $f
    echo "$rest" >> $f
  else
    head -n $LICENSE_LINES "$f" |
    git --no-pager \
      diff --color=always --no-index --exit-code \
      ./LICENSE.inc - 
    return $?
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