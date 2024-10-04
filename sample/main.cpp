// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al

#include <chrono>
#include <stdlib.h>
#include <thread>
#include <uprofile.h>
#if defined(GPU_MONITOR_NVIDIA)
#include <monitors/nvidiamonitor.h>
#endif

void printSystemMemory()
{
    int total = 0, free = 0, available = 0;
    uprofile::getSystemMemory(total, free, available);
    printf("Memory: total = %i MB, free = %i MB, available = %i MB\n", total / 1000, free / 1000, available / 1000);
}

int main(int argc, char* argv[])
{
    uprofile::start("./test.log");

    // --- DUMP CPU USAGE ---
    printf("CPU usage = (");
    std::vector<float> loads = uprofile::getInstantCpuUsage();
    for (auto it = loads.cbegin(); it != loads.cend(); ++it) {
        printf("%0.2f%% ", *it);
    }
    printf(")\n");

    // --- START MONITORING ---
#if defined(GPU_MONITOR_NVIDIA)
    uprofile::addGPUMonitor(new uprofile::NvidiaMonitor);
    uprofile::startGPUMemoryMonitoring(200);
    uprofile::startGPUUsageMonitoring(200);
#endif
    uprofile::startCPUUsageMonitoring(200);
    uprofile::startSystemMemoryMonitoring(200);
    uprofile::startProcessMemoryMonitoring(200);

    // --- USE MEMORY ---
    printSystemMemory();
    uprofile::timeBegin("UseMemory");

    int lengthBuff = 100000000;
    char* buf;
    printf("Allocating %f MB\n", lengthBuff / sizeof(char) / 1000000.0f);
    buf = (char*)malloc(lengthBuff + 1);
    for (int i = 0; i < lengthBuff; i++) {
        buf[i] = rand() % 26 + 'a';
    }
    buf[lengthBuff] = '\0';
    uprofile::timeEnd("UseMemory");
    printSystemMemory();

    // --- WAIT 5 SECONDS ---
    uprofile::timeBegin("Sleep1");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    uprofile::timeEnd("Sleep1");

    // --- RELEASE MEMORY ---
    uprofile::timeBegin("FreeMemory");
    free(buf);
    uprofile::timeEnd("FreeMemory");
    printf("Releasing memory\n");
    printSystemMemory();

    // --- WAIT 5 SECONDS ---
    uprofile::timeBegin("Sleep2");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    uprofile::timeEnd("Sleep2");

    uprofile::stop();

    return 0;
}
