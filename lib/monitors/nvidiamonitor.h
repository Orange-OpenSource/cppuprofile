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

#include "api.h"
#include "igpumonitor.h"
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace uprofile
{
class NvidiaMonitor : public IGPUMonitor
{
public:
    UPROFAPI explicit NvidiaMonitor();
    UPROFAPI virtual ~NvidiaMonitor();

    UPROFAPI void start(int period) override;
    UPROFAPI void stop() override;
    UPROFAPI bool watching() const override;
    UPROFAPI const std::vector<float>& getUsage() const override;
    UPROFAPI void getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem) const override;

private:
    void watchGPU(int period);
    void abortWatchGPU();

    mutable std::mutex m_mutex;
    std::unique_ptr<std::thread> m_watcherThread;
    bool m_watching = false;
    std::vector<int> m_totalMems;
    std::vector<int> m_usedMems;
    std::vector<float> m_gpuUsages;
    size_t m_nbGPUs = 0;
};

}
#endif /* NVIDIAMONITOR_H_ */
