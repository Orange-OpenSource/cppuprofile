// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>

#include "uprofile.h"
#include "uprofileimpl.h"
#include "util/memoryusage.h"

using namespace std::chrono;

namespace uprofile {

void start(const char* file)
{
    UProfileImpl::getInstance()->start(file);
}

void stop()
{
    UProfileImpl::getInstance()->stop();
    UProfileImpl::destroyInstance();
}

void timeBegin(const std::string &step)
{
    UProfileImpl::getInstance()->timeBegin(step);
}

void timeEnd(const std::string &step)
{
    UProfileImpl::getInstance()->timeEnd(step);
}

void startProcessMemoryMonitoring(int period)
{
    UProfileImpl::getInstance()->startProcessMemoryMonitoring(period);
}

void startSystemMemoryMonitoring(int period)
{
    UProfileImpl::getInstance()->startSystemMemoryMonitoring(period);
}

void startCPUUsageMonitoring(int period)
{
    UProfileImpl::getInstance()->startCPUUsageMonitoring(period);
}

void dumpProcessMemory()
{
    UProfileImpl::getInstance()->dumpProcessMemory();
}

void dumpSystemMemory()
{
    UProfileImpl::getInstance()->dumpSystemMemory();
}

void dumpCPUUsage()
{
    UProfileImpl::getInstance()->dumpCPUUsage();
}

}

