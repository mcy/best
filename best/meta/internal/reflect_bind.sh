#!/bin/bash

# Simple shell script that generates the boilerplate for reflect_internal::bind.
#
# Run with best/meta/internal/reflect_bind.sh > best/meta/internal/reflect_bind.inc

MAX_LEN=64

echo "// GENERATED CODE! DO NOT EDIT! See reflect_bind.sh."
echo "#define BEST_REFLECT_MAX_FIELDS_ $MAX_LEN"

# The code below doesn't handle the 0 and 1 cases quite right.
echo "constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<0>)"
echo "requires requires { best::as_auto<decltype(val)>{}; } {"
echo "  return BEST_FWD(cb)();"
echo "}"

echo "constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<1>)"
echo "requires requires { best::as_auto<decltype(val)>{any}; } {"
echo "  auto&& [_0] = BEST_FWD(val);"
echo "  return (BEST_FWD(cb))(BEST_FWD(_0));"
echo "}"

for i in $(seq 1 $(($MAX_LEN - 1))); do
  anys="$(seq 1 $i | xargs printf ', any %.0s')"
  ints="$(seq 1 $i | xargs printf ', _%s')"
  fwds="$(seq 1 $i | xargs printf ', BEST_FWD(_%s)')"
  echo "constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<$(($i + 1))>)"
  echo "requires requires { best::as_auto<decltype(val)>{any$anys}; } {"
  echo "  auto&& [_0$ints] = BEST_FWD(val);"
  echo "  return (BEST_FWD(cb))(BEST_FWD(_0)$fwds);"
  echo "}"
done