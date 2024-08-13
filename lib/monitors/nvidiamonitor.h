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
#include <string>
#include <vector>

using namespace std;

namespace uprofile
{
class NvidiaMonitor : public IGPUMonitor
{
    enum class Data {
        Usage, TotalMem, UsedMem
    };

public:
    UPROFAPI explicit NvidiaMonitor();
    UPROFAPI virtual ~NvidiaMonitor();

    UPROFAPI const std::vector<float>& getUsage() override;
    UPROFAPI void getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem) override;

private:
    void update_gpu_data(const std::vector<Data>& data);

    size_t nGPUs_;
    std::vector<int> m_totalMem;
    std::vector<int> m_usedMem;
    std::vector<float> m_gpuUsage;
};

}
#endif /* NVIDIAMONITOR_H_ */
