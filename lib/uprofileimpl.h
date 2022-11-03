// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef UPROFILEIMPL_H_
#define UPROFILEIMPL_H_

#include <list>
#include <string>
#include <fstream>
#include <map>
#include <mutex>
#include "util/cpumonitor.h"
#include "util/timer.h"

namespace uprofile
{
class UProfileImpl
{
public:
    enum class ProfilingType
    {
        TIME_EXEC,
        TIME_EVENT,
        PROCESS_MEMORY,
        SYSTEM_MEMORY,
        CPU
    };

    static UProfileImpl *getInstance();
    static void destroyInstance();
    virtual ~UProfileImpl();

    // Implementation
    void start(const char *file);
    void stop();
    void timeBegin(const std::string &title);
    void timeEnd(const std::string &title);
    void startProcessMemoryMonitoring(int period);
    void startSystemMemoryMonitoring(int period);
    void startCPUUsageMonitoring(int period);
    void getProcessMemory(int& rss, int& shared);
    void getSystemMemory(int& totalMem, int& availableMem, int& freeMem);
    vector<float> getInstantCpuUsage();

private:
    static UProfileImpl *m_uprofiler;
    UProfileImpl();

    void write(ProfilingType type, const std::list<std::string> &data);
    static int getTimeStamp();

    void dumpCpuUsage();
    void dumpProcessMemory();
    void dumpSystemMemory();

    std::map<std::string, int> m_steps; // Store steps (title, start time)
    std::ofstream m_file;
    Timer m_processMemoryMonitorTimer;
    Timer m_systemMemoryMonitorTimer;
    Timer m_cpuMonitorTimer;
    CpuMonitor m_cpuMonitor;

    std::mutex m_fileMutex;
};

}

#endif /* UPROFILEIMPL_H_ */
