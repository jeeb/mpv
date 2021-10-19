#!/usr/bin/env python3

# Takes an array from meson, sorts it, and prints a string
# separated by spaces to stdout.

import sys
sys.argv.pop(0)
features = sys.argv
features.sort()
features_str = " ".join(features)
sys.stdout.write(features_str)
