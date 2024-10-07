# Software Name : cppuprofile
# SPDX-FileCopyrightText: Copyright (c) 2024 Orange
# SPDX-License-Identifier: BSD-3-Clause
#
# This software is distributed under the BSD License;
# see the LICENSE file for more details.
#
# Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al

import pyuprofile
import time


def alloc_memory():
    """Allocate 1 Gbits of data
    """
    a = bytearray(1000000000)
    time.sleep(3)


def main():
    print("Profiling...")
    output_file = "test.log"
    pyuprofile.start(output_file, 0)
    pyuprofile.set_period(200)
    pyuprofile.time_begin("alloc_mem")
    alloc_memory()
    pyuprofile.time_end("alloc_mem")
    pyuprofile.stop()
    print(f"Profiled events saved into '{output_file}'")


if __name__ == '__main__':
    main()
