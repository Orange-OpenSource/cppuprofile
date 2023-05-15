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

namespace uprofile {

/**
 * Interface to implement for monitoring GPU usage and memory
 *
 * No generic abstraction of GPU metrics exists
 * on Linux nor Windows. So specific IGPUMonitor class should
 * be defined to retrieve metrics from GPU vendor (Nvidia, AMD, Broadcom
 * for RPI...)
 */
class IGPUMonitor {
public:
	virtual ~IGPUMonitor() {}

	// Usage should be in percentage
	virtual float getUsage() = 0;
	// usedMem and totalMem should be returned as KiB
	virtual void getMemory(int& usedMem, int& totalMem) = 0;
};

}

#endif /* IGPUMONITOR_H_ */
