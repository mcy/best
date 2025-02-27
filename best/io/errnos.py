#!/usr/bin/env python3

"""
Generates errnos.inc

Requires e.g. moreutils on Debian. 
"""

import subprocess
import sys

def main():
  raw = subprocess.run(["errno", "-l"], text=True, capture_output=True).stdout

  errnos = []
  for line in raw.splitlines():
    name, num, message = tuple(line.split(" ", 2))
    errnos.append((int(num), name, message))
  
  errnos.sort(key=lambda t: t[0])

  which = sys.argv[1] if len(sys.argv) > 0 else None

  if which == "decls":
    for _, name, _ in errnos:
      print(f"static const ioerr {name.title()};")
    return
  
  if which == "defs":
    for n, name, _ in errnos:
      print(f"constexpr ioerr ioerr::{name.title()}({n});")
    return

  next = 0
  for n, name, message in errnos:
    while next < n:
      print(f"/*{next:04}*/ e{{}},")
      next += 1
    print(f'/*{next:04}*/ e{{"{name}", "{message}"}},')
    next += 1


  next = 0
  for n, name, message in errnos:
    while next < n:
      print(f"/*{next:04}*/ e{{}},")
      next += 1
    print(f'/*{next:04}*/ e{{"{name}", "{message}"}},')
    next += 1

if __name__ == '__main__':
  main()