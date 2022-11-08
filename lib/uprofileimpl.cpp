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
#include <sys/sysinfo.h>

#include "uprofileimpl.h"

#include <unistd.h>

using namespace std::chrono;

namespace uprofile {

UProfileImpl *UProfileImpl::m_uprofiler = NULL;


UProfileImpl::UProfileImpl()
{
}

UProfileImpl *UProfileImpl::getInstance()
{
    if (!m_uprofiler)
    {
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
}

void UProfileImpl::start(const char* file)
{
    m_file.open(file, std::ios::out);
    if (!m_file.is_open())
    {
        std::cerr << "Failed to open file: " << file << std::endl;
    }
}

void UProfileImpl::timeBegin(const std::string& title)
{
    m_steps.insert(make_pair(title, getTimeSinceBoot()));
}

void UProfileImpl::timeEnd(const std::string& title)
{
    // Find step in the map
    auto it = m_steps.find(title);
    if (it != m_steps.end())
    {
        write(ProfilingType::TIME_EXEC, { std::to_string((*it).second), title });
        m_steps.erase(it);
    }
    else
    {
        write(ProfilingType::TIME_EVENT, { title });
    }
}

void UProfileImpl::startProcessMemoryMonitoring(int period)
{
    m_processMemoryMonitorTimer.setInterval(period);
    m_processMemoryMonitorTimer.setTimeout([=](){
        dumpProcessMemory();
    });
    m_processMemoryMonitorTimer.start();
}

void UProfileImpl::startSystemMemoryMonitoring(int period)
{
    m_systemMemoryMonitorTimer.setInterval(period);
    m_systemMemoryMonitorTimer.setTimeout([=](){
        dumpSystemMemory();
    });
    m_systemMemoryMonitorTimer.start();
}

void UProfileImpl::startCPUUsageMonitoring(int period)
{
    m_cpuMonitorTimer.setInterval(period);
    m_cpuMonitorTimer.setTimeout([=](){
        dumpCpuUsage();
    });
    m_cpuMonitorTimer.start();
}

void UProfileImpl::dumpProcessMemory()
{
    int rss = 0, shared = 0;
    getProcessMemory(rss, shared);
    write(ProfilingType::PROCESS_MEMORY, { std::to_string(rss), std::to_string(shared)});
}

void UProfileImpl::dumpSystemMemory()
{
    int total = 0, available = 0, free = 0;
    getSystemMemory(total, available, free);
    write(ProfilingType::SYSTEM_MEMORY, { std::to_string(total), std::to_string(available), std::to_string(free)});
}

void UProfileImpl::dumpCpuUsage()
{
    vector<float> cpuLoads = m_cpuMonitor.getUsage();
    for (size_t index = 0; index < cpuLoads.size(); ++index) {
        write(ProfilingType::CPU, { std::to_string(index), std::to_string(cpuLoads.at(index)) });
    }
}

vector<float> UProfileImpl::getInstantCpuUsage()
{
    // To get instaneous CPU usage, we should wait at least one unit between two polling (aka: 100 ms)
    m_cpuMonitor.getUsage();
    usleep(100000);
    return m_cpuMonitor.getUsage();
}

void UProfileImpl::stop()
{
    m_cpuMonitorTimer.stop();
    m_file.close();
}

void UProfileImpl::write(ProfilingType type, const std::list<std::string>& data)
{
    std::lock_guard<std::mutex> guard(m_fileMutex);
    if (m_file.is_open())
    {
        const char csvSeparator = ';';
        std::string strType;
        switch (type)
        {
            case ProfilingType::TIME_EXEC: strType = "time_exec";
                break;
            case ProfilingType::TIME_EVENT: strType = "time_event";
                break;
            case ProfilingType::PROCESS_MEMORY: strType = "proc_mem";
                break;
            case ProfilingType::SYSTEM_MEMORY: strType = "sys_mem";
                break;
            case ProfilingType::CPU: strType = "cpu";
                break;
            default: strType = "undefined";
                break;
        }
        m_file << strType.c_str() << csvSeparator << getTimeSinceBoot();
        for (auto it = data.cbegin(); it != data.cend(); ++it)
        {
            m_file << csvSeparator << *it;
        }
        m_file << "\n";
        m_file.flush();
    }
}

unsigned long long UProfileImpl::getTimeSinceBoot()
{
    std::chrono::milliseconds uptime(0u);
    double uptime_seconds;
    if (std::ifstream("/proc/uptime", std::ios::in) >> uptime_seconds)
    {
      return static_cast<unsigned long long>(uptime_seconds*1000.0);
    }
    return 0;
}


void UProfileImpl::getSystemMemory(int &totalMem, int &availableMem, int &freeMem)
{
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
        }
        else if (line.find("MemFree") != std::string::npos) {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> freeMem;
        }
        else if (line.find("MemAvailable") != std::string::npos) {
            {
                stringstream ls(line);
                ls.ignore(256, ' ');
                ls >> availableMem;
                break;
            }
        }
    }
}

void UProfileImpl::getProcessMemory(int &rss, int &shared)
{
    int tSize = 0, resident = 0, share = 0;
    ifstream buffer("/proc/self/statm");
    buffer >> tSize >> resident >> share;
    buffer.close();

    long page_size_kb = getpagesize() / 1024;
    rss = resident * page_size_kb;
    shared = share * page_size_kb;
}

}

