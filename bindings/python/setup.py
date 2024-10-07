# Software Name : cppuprofile
# SPDX-FileCopyrightText: Copyright (c) 2024 Orange
# SPDX-License-Identifier: BSD-3-Clause
#
# This software is distributed under the BSD License;
# see the LICENSE file for more details.
#
# Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al

from setuptools import setup, Extension
from cmake_setuptools import *
import os
import subprocess


def get_version():
      """Return the latest tag since setuptool expects a version with the following pattern x.x.x
      """
      return subprocess.check_output(["git", "describe", "--abbrev=0"]).strip().decode()


def build_cmake_macros():
      cmake_common_variables = "-DPROFILE_ENABLED=ON"
      if "CMAKE_COMMON_VARIABLES" in os.environ:
            cmake_common_variables += " {}".format(os.environ["CMAKE_COMMON_VARIABLES"])
      os.environ["CMAKE_COMMON_VARIABLES"] = cmake_common_variables


build_cmake_macros()
setup(name='pyuprofile',
      description='CPU, GPU and memory profiling package',
      version=get_version(),
      ext_modules=[CMakeExtension("pyuprofile")],
      cmdclass={'build_ext': CMakeBuildExt}
)
