// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2023 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef IGPUMONITOR_H_
#define IGPUMONITOR_H_

#include <vector>

namespace uprofile
{

/**
 * Interface to implement for monitoring GPU(s) usage and memory
 *
 * No generic abstraction of GPU metrics exists
 * on Linux nor Windows. So specific IGPUMonitor class should
 * be defined to retrieve metrics from GPU vendor (Nvidia, AMD, Broadcom
 * for RPI...)
 */
class IGPUMonitor
{
public:
    virtual ~IGPUMonitor() {}

    // Start monitoring
    virtual void start(int period) = 0;
    // Stop monitoring
    virtual void stop() = 0;
    // Return if monitor is currently watching data
    virtual bool watching() const = 0;

    // Usages should be in percentage
    virtual const std::vector<float>& getUsage() const = 0;
    // usedMems and totalMems should be returned as KiB
    virtual void getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem) const = 0;
};

}

#endif /* IGPUMONITOR_H_ */
