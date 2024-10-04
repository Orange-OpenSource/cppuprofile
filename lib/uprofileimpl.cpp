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

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "uprofileimpl.h"

using namespace std::chrono;

namespace uprofile
{

UProfileImpl* UProfileImpl::m_uprofiler = NULL;
UProfileImpl::UProfileImpl() :
    m_tsUnit(TimestampUnit::EPOCH_TIME),
    m_gpuMonitor(NULL)
{
}

UProfileImpl* UProfileImpl::getInstance()
{
    if (!m_uprofiler) {
        m_uprofiler = new UProfileImpl;
    }
    return m_uprofiler;
}

void UProfileImpl::destroyInstance()
{
    delete m_uprofiler;
    m_uprofiler = NULL;
}

UProfileImpl::~UProfileImpl()
{
    removeGPUMonitor();
}

void UProfileImpl::start(const char* filepath, unsigned long long maxCapSize)
{
    m_file = make_shared<EventsFile>(filepath, maxCapSize);
}

void UProfileImpl::addGPUMonitor(IGPUMonitor* monitor)
{
    if (!monitor) {
        std::cerr << "Invalid GPU Monitor" << std::endl;
        return;
    }

    removeGPUMonitor();

    m_gpuMonitor = monitor;
}

void UProfileImpl::removeGPUMonitor()
{
    if (m_gpuMonitor) {
        delete m_gpuMonitor;
        m_gpuMonitor = NULL;
    }
}

void UProfileImpl::timeBegin(const std::string& title)
{
    std::lock_guard<std::mutex> guard(m_stepsMutex);
    m_steps.insert(make_pair(title, getTimestamp()));
}

void UProfileImpl::timeEnd(const std::string& title)
{
    unsigned long long beginTimestamp = 0;

    // Find step in the map
    unique_lock<mutex> lk(m_stepsMutex);
    auto it = m_steps.find(title);
    bool found = it != m_steps.end();
    if (found) {
        beginTimestamp = (*it).second;
        m_steps.erase(it);
    }
    lk.unlock();

    if (found) {
        write(ProfilingType::TIME_EXEC, {std::to_string(beginTimestamp), title});
    } else {
        write(ProfilingType::TIME_EVENT, {title});
    }
}

void UProfileImpl::startProcessMemoryMonitoring(int period)
{
    m_processMemoryMonitorTimer.setInterval(period);
    m_processMemoryMonitorTimer.setTimeout([=]() {
        dumpProcessMemory();
    });
    m_processMemoryMonitorTimer.start();
}

void UProfileImpl::startSystemMemoryMonitoring(int period)
{
    m_systemMemoryMonitorTimer.setInterval(period);
    m_systemMemoryMonitorTimer.setTimeout([=]() {
        dumpSystemMemory();
    });
    m_systemMemoryMonitorTimer.start();
}

void UProfileImpl::startCPUUsageMonitoring(int period)
{
    m_cpuMonitorTimer.setInterval(period);
    m_cpuMonitorTimer.setTimeout([=]() {
        dumpCpuUsage();
    });
    m_cpuMonitorTimer.start();
}

void UProfileImpl::startGPUUsageMonitoring(int period)
{
    if (!m_gpuMonitor) {
        std::cerr << "Cannot monitor GPU usage: no GPUMonitor set!" << std::endl;
        return;
    }

    m_gpuMonitor->start(period);

    m_gpuUsageMonitorTimer.setInterval(period);
    m_gpuUsageMonitorTimer.setTimeout([=]() {
        dumpGpuUsage();
    });
    m_gpuUsageMonitorTimer.start();
}

void UProfileImpl::startGPUMemoryMonitoring(int period)
{
    if (!m_gpuMonitor) {
        std::cerr << "Cannot monitor GPU memory: no GPUMonitor set!" << std::endl;
        return;
    }
    m_gpuMonitor->start(period);

    m_gpuMemoryMonitorTimer.setInterval(period);
    m_gpuMemoryMonitorTimer.setTimeout([=]() {
        dumpGpuMemory();
    });
    m_gpuMemoryMonitorTimer.start();
}

void UProfileImpl::dumpProcessMemory()
{
    int rss = 0, shared = 0;
    getProcessMemory(rss, shared);
    write(ProfilingType::PROCESS_MEMORY, {std::to_string(rss), std::to_string(shared)});
}

void UProfileImpl::dumpSystemMemory()
{
    int total = 0, available = 0, free = 0;
    getSystemMemory(total, available, free);
    write(ProfilingType::SYSTEM_MEMORY, {std::to_string(total), std::to_string(available), std::to_string(free)});
}

void UProfileImpl::dumpCpuUsage()
{
    vector<float> cpuLoads = m_cpuMonitor.getUsage();
    for (size_t index = 0; index < cpuLoads.size(); ++index) {
        write(ProfilingType::CPU, {std::to_string(index), std::to_string(cpuLoads.at(index))});
    }
}

void UProfileImpl::dumpGpuUsage()
{
    if (!m_gpuMonitor || !m_gpuMonitor->watching()) {
        return;
    }

    auto const& usage = m_gpuMonitor->getUsage();
    for (size_t i = 0; i < usage.size(); ++i) {
        write(ProfilingType::GPU_USAGE, {std::to_string(i), std::to_string(usage[i])});
    }
}

void UProfileImpl::dumpGpuMemory()
{
    if (!m_gpuMonitor || !m_gpuMonitor->watching()) {
        return;
    }

    vector<int> usedMems, totalMems;
    m_gpuMonitor->getMemory(usedMems, totalMems);
    for (size_t i = 0; i < usedMems.size(); ++i) {
        write(ProfilingType::GPU_MEMORY, {std::to_string(i), std::to_string(usedMems[i]), std::to_string(totalMems[i])});
    }
}

vector<float> UProfileImpl::getInstantCpuUsage()
{
    // To get instaneous CPU usage, we should wait at least one unit between two polling (aka: 100 ms)
    m_cpuMonitor.getUsage();
    this_thread::sleep_for(std::chrono::milliseconds(100));
    return m_cpuMonitor.getUsage();
}

void UProfileImpl::stop()
{
    m_processMemoryMonitorTimer.stop();
    m_systemMemoryMonitorTimer.stop();
    m_cpuMonitorTimer.stop();
    m_gpuUsageMonitorTimer.stop();
    m_gpuMemoryMonitorTimer.stop();
    if (m_gpuMonitor) {
        m_gpuMonitor->stop();
    }
    m_file.reset();
}

void UProfileImpl::setTimestampUnit(TimestampUnit tsUnit)
{
    m_tsUnit = tsUnit;
}

void UProfileImpl::write(ProfilingType type, const std::list<std::string>& data)
{
    if (!m_file) {
        return;
    }
    std::string strType;
    switch (type) {
    case ProfilingType::TIME_EXEC:
        strType = "time_exec";
        break;
    case ProfilingType::TIME_EVENT:
        strType = "time_event";
        break;
    case ProfilingType::PROCESS_MEMORY:
        strType = "proc_mem";
        break;
    case ProfilingType::SYSTEM_MEMORY:
        strType = "sys_mem";
        break;
    case ProfilingType::CPU:
        strType = "cpu";
        break;
    case ProfilingType::GPU_USAGE:
        strType = "gpu";
        break;
    case ProfilingType::GPU_MEMORY:
        strType = "gpu_mem";
        break;
    default:
        strType = "undefined";
        break;
    }
    m_file->write(strType, getTimestamp(), data);
}

unsigned long long UProfileImpl::getTimestamp() const
{
    return (m_tsUnit == TimestampUnit::EPOCH_TIME ? getEpochTime() : getTimeSinceBoot());
}

unsigned long long UProfileImpl::getEpochTime()
{
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

unsigned long long UProfileImpl::getTimeSinceBoot()
{
#if defined(__linux__)
    double uptime_seconds;
    if (std::ifstream("/proc/uptime", std::ios::in) >> uptime_seconds) {
        return static_cast<unsigned long long>(uptime_seconds * 1000.0);
    }
    return 0;
#elif defined(_WIN32)
    return GetTickCount64();
#else
    return 0;
#endif
}

void UProfileImpl::getSystemMemory(int& totalMem, int& availableMem, int& freeMem)
{

#if defined(__linux__)
    // /proc/meminfo returns the dump here:
    // MemTotal: 515164 kB
    // MemFree: 7348 kB
    // MemAvailable: 7348 kB
    ifstream meminfo("/proc/meminfo");
    string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal") != std::string::npos) {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> totalMem;
        } else if (line.find("MemFree") != std::string::npos) {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> freeMem;
        } else if (line.find("MemAvailable") != std::string::npos) {
            {
                stringstream ls(line);
                ls.ignore(256, ' ');
                ls >> availableMem;
                break;
            }
        }
    }
#endif
}

void UProfileImpl::getProcessMemory(int& rss, int& shared)
{

#if defined(__linux__)
    int tSize = 0, resident = 0, share = 0;
    ifstream buffer("/proc/self/statm");
    buffer >> tSize >> resident >> share;
    buffer.close();

    long page_size_kb = getpagesize() / 1024;
    rss = resident * page_size_kb;
    shared = share * page_size_kb;
#endif
}

}
