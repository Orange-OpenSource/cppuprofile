// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2023 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "nvidiamonitor.h"

#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string.h>

#if defined(__linux__)
#include <sys/wait.h>
#include <unistd.h>
#endif

const string errorMsg = "Failed to monitor nvidia-smi process";

#if defined(__linux__)
int read_nvidia_smi_stdout(int fd, string& gpuUsage, string& usedMem, string& totalMem)
{
    string line;
    while (line.find('\n') == string::npos) { // full line read
        char buffer[4096];
        ssize_t count = read(fd, buffer, sizeof(buffer)); // if child process crashes, we gonna be blocked here forever
        if (count == -1) {
            return errno;
        } else if (count > 0) { // there is something to read
            line += string(buffer, count);
        }
    }

    // Remove colon to have only spaces and use istringstream
    auto noSpaceEnd = remove(line.begin(), line.end(), ',');
    if (noSpaceEnd == line.end()) { // output trace does not have comma so something went wrong with the command
        return ENODATA;
    }

    line.erase(noSpaceEnd, line.end());
    std::istringstream ss(line);
    ss >> gpuUsage >> usedMem >> totalMem;

    return 0;
}
#endif

uprofile::NvidiaMonitor::NvidiaMonitor()
{
}

uprofile::NvidiaMonitor::~NvidiaMonitor()
{
    stop();
}

void uprofile::NvidiaMonitor::start(int period)
{
    watchGPU(period);
}

void uprofile::NvidiaMonitor::stop()
{
    abortWatchGPU();
}

float uprofile::NvidiaMonitor::getUsage()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gpuUsage;
}

void uprofile::NvidiaMonitor::getMemory(int& usedMem, int& totalMem)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    usedMem = m_usedMem;
    totalMem = m_totalMem;
}

void uprofile::NvidiaMonitor::watchGPU(int period)
{
    if (m_watching) {
        return;
    }

#if defined(__linux__)
    char* args[5];
    args[0] = (char*)"/usr/bin/nvidia-smi";
    string period_arg = "-lms=" + to_string(period); // lms stands for continuous watching
    args[1] = (char*)period_arg.c_str();
    args[2] = (char*)"--query-gpu=utilization.gpu,memory.used,memory.total";
    args[3] = (char*)"--format=csv,noheader,nounits";
    args[4] = NULL;
    string output;
    int pipes[2];

    // Create the pipe
    if (pipe(pipes) == -1) {
        cerr << errorMsg << ": pipe creation failed" << endl;
        return;
    }

    // Create a child process for calling nvidia-smi
    pid_t pid = fork();

    switch (pid) {
    case -1: /* Error */
        cerr << errorMsg << ": process fork failed" << endl;
        return;
    case 0: /* We are in the child process */
        while ((dup2(pipes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {
        }
        close(pipes[1]);
        close(pipes[0]);
        execv(args[0], args);
        cerr << "Failed to execute '" << args[0] << "': " << strerror(errno) << endl; /* execl doesn't return unless there's an error */
        exit(1);
    default: /* We are in the parent process */
        int stdout_fd = pipes[0];

        // Start a thread to retrieve the child process stdout
        m_watching = true;
        m_watcherThread = unique_ptr<std::thread>(new thread([stdout_fd, pid, this]() {
            while (shouldWatch()) {
                string gpuUsage, usedMem, totalMem;
                // if the child process crashes, an error is raised here and threads ends up
                int err = read_nvidia_smi_stdout(stdout_fd, gpuUsage, usedMem, totalMem);
                if (err != 0) {
                    cerr << errorMsg << ": read_error = " << strerror(err) << endl;
                    break;
                }
                m_mutex.lock();
                m_gpuUsage = !gpuUsage.empty() ? stof(gpuUsage) : 0.f;
                m_usedMem = !usedMem.empty() ? stoi(usedMem) * 1024 : 0;    // MiB to KiB
                m_totalMem = !totalMem.empty() ? stoi(totalMem) * 1024 : 0; // MiB to KiB
                m_mutex.unlock();
            }
        }));
    }
#else
    cerr << errorMsg << endl;
#endif
}

void uprofile::NvidiaMonitor::abortWatchGPU()
{
#if defined(__linux__)
    if (m_watcherThread) {
        m_mutex.lock();
        m_watching = false;
        m_mutex.unlock();
        m_watcherThread->join();
        m_watcherThread.reset();
    }
#endif
}

bool uprofile::NvidiaMonitor::shouldWatch()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_watching;
}
