// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2024 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "uprofile.h"
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>

#if defined(GPU_MONITOR_NVIDIA)
#include <monitors/nvidiamonitor.h>
#endif

namespace py = pybind11;

void setPeriod(int period)
{
#if defined(GPU_MONITOR_NVIDIA)
    uprofile::addGPUMonitor(new uprofile::NvidiaMonitor);
    uprofile::startGPUUsageMonitoring(period);
    uprofile::startGPUMemoryMonitoring(period);
#endif
    uprofile::startCPUUsageMonitoring(period);
    uprofile::startProcessMemoryMonitoring(period);
    uprofile::startSystemMemoryMonitoring(period);
}

PYBIND11_MODULE(pyuprofile, m)
{
    m.doc() = "Python uprofile monitoring package"; // optional module docstring

    // Redirect C++ stderr to Python stderr
    py::scoped_ostream_redirect output{
        std::cerr, py::module::import("sys").attr("stderr")};

    // Make Python bindings simpler than C++ APIs
    m.def("start", &uprofile::start, "Start profiling");
    m.def("stop", &uprofile::stop, "Stop profiling");
    m.def("time_begin", &uprofile::timeBegin, "Start recording an event");
    m.def("time_end", &uprofile::timeEnd, "Stop recording an event");
    m.def("set_period", setPeriod);
}
