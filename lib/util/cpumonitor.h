// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef CPUMONITOR_H_
#define CPUMONITOR_H_

#include <string>
#include <vector>

using namespace std;

namespace uprofile
{

class CpuMonitor
{
public:
    explicit CpuMonitor();
    virtual ~CpuMonitor();

    vector<float> getUsage();

private:
    static size_t getNumberOfCPUCores();
    static void extractCpuTimes(const string& cpuInfo, size_t& idleTime, size_t& totalTime);

    size_t m_nbCpus;
    // Store the last idle and total time for each CPU
    vector<size_t> m_lastIdleTimes;
    vector<size_t> m_lastTotalTimes;
};
}

#endif /* CPUMONITOR_H_ */
