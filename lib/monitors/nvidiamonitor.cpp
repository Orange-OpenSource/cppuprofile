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
#include <array>

#if defined(__linux__)
#include <sys/wait.h>
#include <unistd.h>
#endif

const string errorMsg = "Failed to monitor nvidia-smi process";

#if defined(__linux__)
int read_nvidia_smi_stdout(int fd, size_t nGPUs, std::vector<string>& gpuUsage, std::vector<string>& usedMem, std::vector<string>& totalMem)
{
    std::vector<string> lines;
    string leftover;
    string line;
    for (size_t i = 0; i < nGPUs; ++i) { //read in one full line for every gpu in the system
        char buffer[4096];
        ssize_t count = read(fd, buffer, sizeof(buffer)); // if child process crashes, we gonna be blocked here forever
        if (count == -1) {
            return errno;
        } else if (count > 0) { // there is something to read
            string data(buffer, count);
            data = leftover + data;  // Prepend leftover from last read (sometimes vreaks up lines...)

            size_t pos = 0;
            while ((pos = data.find('\n')) != std::string::npos) {
                string line = data.substr(0, pos);
                lines.push_back(line);
                data.erase(0, pos + 1);
            }
            leftover = data;  // Save any remaining part for the next read
        }        
    }
    for (auto& line : lines) {
        // Remove colon to have only spaces and use istringstream
        auto noSpaceEnd = remove(line.begin(), line.end(), ',');
        if (noSpaceEnd == line.end()) { // output trace does not have comma so something went wrong with the command
            return ENODATA;
        }

        line.erase(noSpaceEnd, line.end());
        std::istringstream ss(line);
        std::string gpuIdx, temp_gpuUsage, temp_usedMem, temp_totalMem;
        ss >> gpuIdx >> temp_gpuUsage >> temp_usedMem >> temp_totalMem;
        size_t idx = static_cast<size_t>(std::stoull(gpuIdx));
        if (idx >= nGPUs) {
            std::cerr << "GPU index out of range: " << idx << std::endl;
            continue;  // Handle the error appropriately
        }
        gpuUsage[idx] = temp_gpuUsage; 
        usedMem[idx] = temp_usedMem; 
        totalMem[idx] = temp_totalMem;
    }   

    return 0;
}
#endif

uprofile::NvidiaMonitor::NvidiaMonitor()
{
    //query nvidia-smi to get the number of GPUs and initialize m_totalMem, m_usedMem, and m_gpuUsage vectors 
    #if defined(__linux__)
        try {
            std::array<char, 128> buffer;
            std::string result;
            std::string cmd = "/usr/bin/nvidia-smi --query-gpu=count --format=csv,noheader,nounits";
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            nGPUs_ = static_cast<size_t>(std::stoull(result));
            m_totalMem = std::vector<int>(nGPUs_, 0);
            m_usedMem = std::vector<int>(nGPUs_, 0);
            m_gpuUsage = std::vector<float>(nGPUs_, 0.0);
        } catch (const std::exception& err) {
            std::cerr << errorMsg << std::endl;
        }
    #endif
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
    return m_gpuUsage;
}

void uprofile::NvidiaMonitor::getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem) const
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
                std::vector<string> gpuUsage(nGPUs_, ""), usedMem(nGPUs_, ""), totalMem(nGPUs_, "");
                // if the child process crashes, an error is raised here and threads ends up
                int err = read_nvidia_smi_stdout(stdout_fd, nGPUs_, gpuUsage, usedMem, totalMem);
                if (err != 0) {
                    cerr << errorMsg << ": read_error = " << strerror(err) << endl;
                    unique_lock<mutex> lk(m_mutex);
                    m_watching = false;
                    lk.unlock();
                    break;
                }

                unique_lock<mutex> lk(m_mutex);
                for (size_t i = 0; i < nGPUs_; ++i) {
                    m_gpuUsage[i] = !gpuUsage[i].empty() ? stof(gpuUsage[i]) : 0.f;
                    m_usedMem[i] = !usedMem[i].empty() ? stoi(usedMem[i]) * 1024 : 0;    // MiB to KiB
                    m_totalMem[i] = !totalMem[i].empty() ? stoi(totalMem[i]) * 1024 : 0; // MiB to KiB
                    lk.unlock();
                }
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
