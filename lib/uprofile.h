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
#include <vector>

// UPROFAPI is used to export public API functions from the DLL / shared library.
#if defined(_UPROFILE_BUILD_SHARED)
#if defined(_WIN32)
/* Build as a Win32 DLL */
#define UPROFAPI __declspec(dllexport)
#elif defined(__linux__)
/* Build as a shared library */
#define UPROFAPI __attribute__((visibility("default")))
#endif // if defined(_UPROFILE_BUILD_SHARED)

#elif defined(UPROFILE_DLL)
#if defined(_WIN32)
/* Call uprofile as a Win32 DLL */
#define UPROFAPI __declspec(dllimport)
#endif // if defined(_WIN32)
#endif // if defined(UPROFILE_DLL)

#if !defined(UPROFAPI)
#define UPROFAPI
#endif

namespace uprofile
{
    /**
     * @brief start
     * Start profiling to record steps.
     * Profiling report will be written in filepath given
     */
	UPROFAPI void start(const char* file);

    /**
     * @brief stop
     * Stop profiling and flush recorded steps to TRACE file
     *
     * Stopping profiler on no started profiler has no effect
     *
     */
	UPROFAPI void stop();

    /**
     * @brief timeBegin
     * @param title
     *
     * Record a new step with its timestamp since profiler start.
     */
	UPROFAPI void timeBegin(const std::string& title);

    /**
     * @brief timeEnd
     * Compute duration between time start and time end of same step
     * If no timeStart has been called before, time start is profiler init time
     */
	UPROFAPI void timeEnd(const std::string& title);

    /**
     * @brief startProcessMemoryMonitoring
     * @param period: period between two memory dump (in ms)
     */
	UPROFAPI void startProcessMemoryMonitoring(int period);

    /**
     * @brief startSystemMemoryMonitoring
     * @param period: period between two memory dump (in ms)
     */
	UPROFAPI void startSystemMemoryMonitoring(int period);

    /**
     * @brief startCPUUsageMonitoring
     * @param period: period between two cpu usage dump (in ms)
     */
	UPROFAPI void startCPUUsageMonitoring(int period);

    /**
     * @brief getProcessMemory: return memory used by the current process
     */
	UPROFAPI void getProcessMemory(int& rss, int& shared);

    /**
     * @brief getSystemMemory: dump global system memory
     */
	UPROFAPI void getSystemMemory(int& totalMem, int& availableMem, int& freeMem);

    /**
     * @brief getInstantCpuUsage: get usage of all cpu cores
     */
	UPROFAPI std::vector<float> getInstantCpuUsage();
}

#endif /* UPROFILE_H_ */
