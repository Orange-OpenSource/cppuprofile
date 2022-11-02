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

#include "uprofileimpl.h"
#include "util/memoryusage.h"

using namespace std::chrono;

namespace uprofile {

UProfileImpl *UProfileImpl::m_uprofiler = NULL;


UProfileImpl::UProfileImpl():
    m_cpuPreviousIdleTime(0),
    m_cpuPreviousTotalTime(0)
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
    m_steps.insert(make_pair(title, getTimeStamp()));
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
}

void UProfileImpl::startSystemMemoryMonitoring(int period)
{
}

void UProfileImpl::startCPUUsageMonitoring(int period)
{
}

void UProfileImpl::dumpProcessMemory()
{
    int rss, shared;
    getProcessMemory(rss, shared);
    write(ProfilingType::PROCESS_MEMORY, { std::to_string(rss), std::to_string(shared)});
}

void UProfileImpl::dumpSystemMemory()
{
    int total = 0, available = 0, free = 0;
    getSystemMemory(total, available, free);
    write(ProfilingType::SYSTEM_MEMORY, { std::to_string(total), std::to_string(available), std::to_string(free)});
}

void UProfileImpl::dumpCPUUsage()
{
}

void UProfileImpl::stop()
{
    m_file.close();
}

void UProfileImpl::write(ProfilingType type, const std::list<std::string>& data)
{
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
        m_file << strType.c_str() << csvSeparator << getTimeStamp() << csvSeparator;
        for (auto str: data)
        {
            m_file << str << csvSeparator;
        }
        m_file << "\n";
        m_file.flush();
    }
}

int UProfileImpl::getTimeStamp()
{ 
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

}

