// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2023 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef NVIDIAMONITOR_H_
#define NVIDIAMONITOR_H_

#include "igpumonitor.h"
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;

namespace uprofile
{
class NvidiaMonitor : public IGPUMonitor
{
public:
    explicit NvidiaMonitor();
    virtual ~NvidiaMonitor();

    void start(int period) override;
    void stop() override;
    float getUsage() override;
    void getMemory(int& usedMem, int& totalMem) override;

private:
    void watchGPU(int period);
    void abortWatchGPU();
    bool shouldWatch();

    std::mutex m_mutex;
    std::unique_ptr<std::thread> m_watcherThread;
    bool m_watching = false;
    int m_totalMem = 0;
    int m_usedMem = 0;
    float m_gpuUsage = 0.f;
};

}
#endif /* NVIDIAMONITOR_H_ */
