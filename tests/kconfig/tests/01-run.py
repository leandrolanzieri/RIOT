#!/usr/bin/env python3

# Copyright (C) 2019 HAW Hamburg
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import sys
import os
from testrunner import run


def testfunc(child):
    config_file = os.path.basename(os.environ.get("KCONFIG_OVERLAY"))
    print("Configuration overlay: {}".format(config_file))
    child.expect_exact("Message 1 defined in {} file".format(config_file))
    child.expect_exact("MSG_2 is active")


if __name__ == "__main__":
    sys.exit(run(testfunc))
