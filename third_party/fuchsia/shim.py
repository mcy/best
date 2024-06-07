#!/usr/bin/env python3

# Shim over //third_party/fuchsia/check-header-guards.py.

import os
import sys
from third_party.fuchsia.check_header_guards import main

if __name__ == '__main__':
  os.chdir(os.environ['BUILD_WORKING_DIRECTORY'])
  sys.exit(main())