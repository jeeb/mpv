#!/usr/bin/env python3

import os
import sys

toolsdir = os.path.dirname(os.path.abspath(sys.argv[0]))
srcdir = os.path.dirname(toolsdir)
client_h = os.path.join(srcdir, "libmpv", "client.h")

with open(client_h, "r") as f:
    for line in f:
        if "#define MPV_CLIENT_API_VERSION" in line:
            define_line = line
            break

split = define_line.split(",")
major = split[0][-1]
minor = split[1][1:4]
sys.stdout.write(major + "." + minor + ".0")
