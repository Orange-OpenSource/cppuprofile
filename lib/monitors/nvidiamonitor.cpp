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
#include <memory>

using namespace std;

const string nvidiaSmiCmdName = "/usr/bin/nvidia-smi";
const string errorMsg = "Failed to monitor nvidia-smi process";

struct RawMetric {
    string index;
    string usage;
    string usedMem;
    string totalMem;
};

#if defined(__linux__)
int read_nvidia_smi_stdout(int fd, vector<RawMetric>& metrics)
{
    size_t nbCollected = 0;
    while (nbCollected < metrics.size()) {
        string line;

        // nvidia-smi dumps metrics for each GPU line by line
        // so read the stdout line by line and fill the input metrics buffer
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
        istringstream ss(line);
        RawMetric metric;
        ss >> metric.index >> metric.usage >> metric.usedMem >> metric.totalMem;
        metrics[nbCollected] = metric;
        nbCollected++;
    }

    return 0;
}
#endif

uprofile::NvidiaMonitor::NvidiaMonitor()
{
    // Place nvidia-smi command to retrieve number of GPUs
    // to initialize usage and memsvectors
    try {
        char buffer[128];
        string result = "";
        string cmd = nvidiaSmiCmdName;
        cmd += " --query-gpu=count --format=csv,noheader,nounits";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            throw runtime_error("popen() failed!");
        }
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        m_nbGPUs = static_cast<size_t>(std::stoull(result));
        m_totalMems = vector<int>(m_nbGPUs, 0);
        m_usedMems = vector<int>(m_nbGPUs, 0);
        m_gpuUsages = vector<float>(m_nbGPUs, 0.0);
    } catch (const exception& err) {
        cerr << errorMsg << endl;
    }
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

const std::vector<float>& uprofile::NvidiaMonitor::getUsage() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gpuUsages;
}

void uprofile::NvidiaMonitor::getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    usedMem = m_usedMems;
    totalMem = m_totalMems;
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
    args[2] = (char*)"--query-gpu=index,utilization.gpu,memory.used,memory.total";
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
            while (watching()) {
                vector<RawMetric> metrics(m_nbGPUs);
                // if the child process crashes, an error is raised here and threads ends up
                int err = read_nvidia_smi_stdout(stdout_fd, metrics);
                if (err != 0) {
                    cerr << errorMsg << ": read_error = " << strerror(err) << endl;
                    unique_lock<mutex> lk(m_mutex);
                    m_watching = false;
                    lk.unlock();
                    break;
                }

                unique_lock<mutex> lk(m_mutex);
                for (size_t i = 0; i < metrics.size(); ++i) {
                    const auto& m = metrics[i];
                    size_t idx = stoull(m.index);
                    m_gpuUsages[idx] = !m.usage.empty() ? stof(m.usage) : 0.f;
                    m_usedMems[idx] = !m.usedMem.empty() ? stoi(m.usedMem) * 1024 : 0;    // MiB to KiB
                    m_totalMems[idx] = !m.totalMem.empty() ? stoi(m.totalMem) * 1024 : 0; // MiB to KiB
                }
                lk.unlock();
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
        unique_lock<mutex> lk(m_mutex);
        m_watching = false;
        lk.unlock();
        m_watcherThread->join();
        m_watcherThread.reset();
    }
#endif
}

bool uprofile::NvidiaMonitor::watching() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_watching;
}
