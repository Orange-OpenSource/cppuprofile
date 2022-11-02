// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef UPROFILE_H_
#define UPROFILE_H_

#include <list>
#include <string>
#include <fstream>
#include <map>

namespace uprofile
{
    /**
     * @brief start
     * Start profiling to record steps.
     * Profiling report will be written in filepath given
     */
    void start(const char *file);

    /**
     * @brief stop
     * Stop profiling and flush recorded steps to TRACE file
     *
     * Stopping profiler on no started profiler has no effect
     *
     */
    void stop();

    /**
     * @brief timeBegin
     * @param title
     *
     * Record a new step with its timestamp since profiler start.
     */
    void timeBegin(const std::string &title);

    /**
     * @brief timeEnd
     * Compute duration between time start and time end of same step
     * If no timeStart has been called before, time start is profiler init time
     */
    void timeEnd(const std::string &title);

    /**
     * @brief startProcessMemoryMonitoring
     * @param period: period between two memory dump (in ms)
     */
    void startProcessMemoryMonitoring(int period);

    /**
     * @brief startSystemMemoryMonitoring
     * @param period: period between two memory dump (in ms)
     */
    void startSystemMemoryMonitoring(int period);

    /**
     * @brief startCPUUsageMonitoring
     * @param period: period between two cpu usage dump (in ms)
     */
    void startCPUUsageMonitoring(int period);

    /**
     * @brief dumpProcessMemory: dump memory used by current process
     */
    void dumpProcessMemory();

    /**
     * @brief dumpSystemMemory: dump global system memory used by current process
     */
    void dumpSystemMemory();

    /**
     * @brief dumpCPUUsage: dump cpu usage
     */
    void dumpCPUUsage();
}

#endif /* UPROFILE_H_ */
