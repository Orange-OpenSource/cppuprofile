// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef UPROFILEIMPL_H_
#define UPROFILEIMPL_H_

#include <list>
#include <string>
#include <fstream>
#include <map>

namespace uprofile
{
    class UProfileImpl
    {
    public:
        enum class ProfilingType
        {
            TIME_EXEC,
            TIME_EVENT,
            PROCESS_MEMORY,
            SYSTEM_MEMORY,
            CPU
        };

        static UProfileImpl *getInstance();
        static void destroyInstance();
        virtual ~UProfileImpl();

        // Implementation
        void start(const char *file);
        void stop();
        void timeBegin(const std::string &title);
        void timeEnd(const std::string &title);
        void startProcessMemoryMonitoring(int period);
        void startSystemMemoryMonitoring(int period);
        void startCPUUsageMonitoring(int period);
        void dumpProcessMemory();
        void dumpSystemMemory();
        void dumpCPUUsage();

    private:
        static UProfileImpl *m_uprofiler;
        UProfileImpl();

        void write(ProfilingType type, const std::list<std::string> &data);
        static int getTimeStamp();

        std::map<std::string, int> m_steps; // Store steps (title, start time)
        std::ofstream m_file;
        /*    QTimer m_systemMemoryMonitorTimer;
           QTimer m_processMemoryMonitorTimer;
           QTimer m_cpuUsageMonitorTimer;
         */
        size_t m_cpuPreviousIdleTime;
        size_t m_cpuPreviousTotalTime;
    };

}

#endif /* UPROFILEIMPL_H_ */
