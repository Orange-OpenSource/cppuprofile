// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include "uprofile.h"
#include "uprofileimpl.h"

using namespace std::chrono;

#ifdef PROFILE_ON
#define UPROFILE_INSTANCE_CALL(func, ...) \
    UProfileImpl::getInstance()->func(__VA_ARGS__);
#define UPROFILE_INSTANCE_CALL_RETURN(func, ...) \
    return UProfileImpl::getInstance()->func(__VA_ARGS__);
#define UPROFILE_DESTROY_INSTANCE() \
    UProfileImpl::destroyInstance();
#else
#define UPROFILE_INSTANCE_CALL(func, ...) (void)0;
#define UPROFILE_INSTANCE_CALL_RETURN(func, ...) \
    return {}
#define UPROFILE_DESTROY_INSTANCE() (void)0;
#endif

namespace uprofile
{

void start(const char* filepath, unsigned long long maxCapSize)
{
    UPROFILE_INSTANCE_CALL(start, filepath, maxCapSize);
}

void stop()
{
    UPROFILE_INSTANCE_CALL(stop);
    UPROFILE_DESTROY_INSTANCE();
}

void addGPUMonitor(IGPUMonitor* monitor)
{
    UPROFILE_INSTANCE_CALL(addGPUMonitor, monitor);
}

void removeGPUMonitor()
{
    UPROFILE_INSTANCE_CALL(removeGPUMonitor);
}

void setTimestampUnit(TimestampUnit tsUnit)
{
    UPROFILE_INSTANCE_CALL(setTimestampUnit, tsUnit);
}

void timeBegin(const std::string& step)
{
    UPROFILE_INSTANCE_CALL(timeBegin, step);
}

void timeEnd(const std::string& step)
{
    UPROFILE_INSTANCE_CALL(timeEnd, step);
}

void startProcessMemoryMonitoring(int period)
{
    UPROFILE_INSTANCE_CALL(startProcessMemoryMonitoring, period);
}

void startSystemMemoryMonitoring(int period)
{
    UPROFILE_INSTANCE_CALL(startSystemMemoryMonitoring, period);
}

void startCPUUsageMonitoring(int period)
{
    UPROFILE_INSTANCE_CALL(startCPUUsageMonitoring, period);
}

void startGPUUsageMonitoring(int period)
{
    UPROFILE_INSTANCE_CALL(startGPUUsageMonitoring, period);
}

void startGPUMemoryMonitoring(int period)
{
    UPROFILE_INSTANCE_CALL(startGPUMemoryMonitoring, period);
}

void getProcessMemory(int& rss, int& shared)
{
    UPROFILE_INSTANCE_CALL(getProcessMemory, rss, shared);
}

void getSystemMemory(int& totalMem, int& availableMem, int& freeMem)
{
    UPROFILE_INSTANCE_CALL(getSystemMemory, totalMem, availableMem, freeMem);
}

std::vector<float> getInstantCpuUsage()
{
    UPROFILE_INSTANCE_CALL_RETURN(getInstantCpuUsage);
}

}
