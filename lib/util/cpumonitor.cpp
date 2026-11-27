// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "cpumonitor.h"
#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

uprofile::CpuMonitor::CpuMonitor() :
    m_nbCpus(getNumberOfCPUCores()),
    m_lastIdleTimes(m_nbCpus, 0),
    m_lastTotalTimes(m_nbCpus, 0)
{
}

uprofile::CpuMonitor::~CpuMonitor()
{
}

size_t uprofile::CpuMonitor::getNumberOfCPUCores()
{
    size_t nbCores = 0;
#if defined(__linux__)
    ifstream meminfo("/proc/cpuinfo");
    string str;
    while (getline(meminfo, str)) {
        if (str.rfind("processor", 0) == 0) {
            nbCores++;
        }
    }
#endif
    return nbCores;
}

vector<float> uprofile::CpuMonitor::getUsage()
{
    std::regex pattern(R"(^(cpu\d+)\s+(.*)$)");
    std::smatch match;
    vector<float> usages(m_nbCpus, 0);
#if defined(__linux__)
    ifstream procStat("/proc/stat");
    // /proc/stat dumps the following info:
    //      user nice system idle iowait irq softirq
    // cpu  2255 34 2290 22625563 6290 127 456
    // cpu0 1132 34 1441 11311718 3675 127 438
    // cpu1 1123 0 849 11313845 2614 0 18
    // ...
    // Each number represents the amount of time the CPU has spent performing
    // different kinds of work
    // Extract from /proc/stat the CPU idle time and tht total time
    string line;
    while (getline(procStat, line)) {
        if (std::regex_search(line, match, pattern)) {
            size_t cpuIndex = stoi(match[1].str().substr(3)); // Extract the CPU index from "cpuX"
            size_t idleTime = 0, totalTime = 0;
            std::stringstream times(match[2].str());
            int index = 0;
            for (size_t time = 0; times >> time; ++index) {
                if (index == 3) { // idle time is the 4th param
                    idleTime = time;
                }
                totalTime += time;
            }

            // To compute CPU load, we compute the time the CPU has been idle since the last read.
            float cpuLoad = 100.0 * (1.0 - (float)(idleTime - m_lastIdleTimes[cpuIndex]) / (totalTime - m_lastTotalTimes[cpuIndex]));
            usages[cpuIndex] = cpuLoad;

            // Save the times value for the next read
            m_lastIdleTimes[cpuIndex] = idleTime;
            m_lastTotalTimes[cpuIndex] = totalTime;
        }
    }
#endif

    return usages;
}
