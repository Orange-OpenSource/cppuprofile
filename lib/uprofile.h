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

#include <fstream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "api.h"
#include "igpumonitor.h"
#include "timestampunit.h"

/**
 * @defgroup uprofile Functions for monitoring system metrics
 *  @{
 */
namespace uprofile
{
/**
 * @ingroup uprofile
 * @brief Start profiling to periodically record monitored events to a file
 * @param filepath: file path where events will be saved
 * @param maxCapSize: maximum file size in bytes
 *
 * By default, the file is unbounded.
 *
 * If you have some storage constraints, set maxCapSize parameter for generating two rotating files (<file>_0.<ext> and <file>_1.<ext>)
 * Each file will have a maximum size of maxCapSize/2 (In this mode, more recent events will override older events)
 */
UPROFAPI void start(const char* filepath, unsigned long long maxCapSize = 0);

/**
 * @ingroup uprofile
 * @brief Stop all monitorings
 *
 * Note: stopping profiler without prior call to start() has no effect
 */
UPROFAPI void stop();

/**
 * @ingroup uprofile
 * @brief Inject a GPUMonitor object that will be responsible for monitoring GPU metrics (usage and memory)
 * @param monitor: custom GPUMonitor object
 *
 * Note: uprofile takes ownership of the passed object.
 */
UPROFAPI void addGPUMonitor(IGPUMonitor* monitor);

/**
 * @ingroup uprofile
 * @brief Destroy injected GPUMonitor object
 */
UPROFAPI void removeGPUMonitor();

/**
 * @ingroup uprofile
 * @brief Change the timestamp unit to record profiling metrics
 *
 * It should be called before calling start() method.
 *
 * Note: default value is EPOCH_TIME
 */
UPROFAPI void setTimestampUnit(TimestampUnit tsUnit);

/**
 * @ingroup uprofile
 * @brief Start monitoring the execution time of the given event
 * @param title: event key
 */
UPROFAPI void timeBegin(const std::string& title);

/**
 * @ingroup uprofile
 * @brief Stop monitoring the execution time of the given event
 *
 * The library computes the duration for the given event and saves it into the report file.
 *
 * If no timeBegin() has been called with the given title, the call is ignored.
 */
UPROFAPI void timeEnd(const std::string& title);

/**
 * @ingroup uprofile
 * @brief Start monitoring of the memory used by the process
 * @param period: period between two memory dump (in ms)
 */
UPROFAPI void startProcessMemoryMonitoring(int period);

/**
 * @ingroup uprofile
 * @brief Start monitoring of the global memory used on the system
 * @param period: period between two memory dump (in ms)
 */
UPROFAPI void startSystemMemoryMonitoring(int period);

/**
 * @ingroup uprofile
 * @brief Start monitoring of the usage percentage of each CPU
 * @param period: period between two cpu usage dump (in ms)
 */
UPROFAPI void startCPUUsageMonitoring(int period);

/**
 * @ingroup uprofile
 * @brief Start monitoring of the usage of the GPU
 * @param period: period between two gpu usage dump (in ms)
 */
UPROFAPI void startGPUUsageMonitoring(int period);

/**
 * @ingroup uprofile
 * @brief Start monitoring of the usage of the GPU memory
 * @param period: period between two gpu usage dump (in ms)
 */
UPROFAPI void startGPUMemoryMonitoring(int period);

/**
 * @ingroup uprofile
 * @brief memory used by the current process
 */
UPROFAPI void getProcessMemory(int& rss, int& shared);

/**
 * @ingroup uprofile
 * @brief dump global system memory
 */
UPROFAPI void getSystemMemory(int& totalMem, int& availableMem, int& freeMem);

/**
 * @ingroup uprofile
 * @brief get usage of all cpu cores
 * @return vector holding the usage percentage of each CPU core
 */
UPROFAPI std::vector<float> getInstantCpuUsage();
}
/** @} */ // end of uprofile group

#endif /* UPROFILE_H_ */
