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
#include <iostream>
#include <sstream>
#include <memory>

const string errorMsg = "Failed to monitor nvidia-smi process";

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
{}

void uprofile::NvidiaMonitor::update_gpu_data(const std::vector<uprofile::NvidiaMonitor::Data>& data) {
    #if defined(__linux__)
        try {
            //invoke nvidia-smi to collec
            std::array<char, 128> buffer;
            std::string result;
            std::string cmd = "/usr/bin/nvidia-smi --query-gpu=index";
            for (auto const & d : data) {
                switch (d) {
                    case uprofile::NvidiaMonitor::Data::Usage: {
                        cmd += ",utilization.gpu";
                        break;  
                    };
                    case uprofile::NvidiaMonitor::Data::UsedMem: {
                        cmd += ",memory.used";
                        break;  
                    };
                    case uprofile::NvidiaMonitor::Data::TotalMem: {
                        cmd += ",memory.total";
                        break;  
                    };
                }
            }
            cmd += " --format=csv,noheader,nounits";   
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }

            std::istringstream ss(result); 
            std::string line; 
            while (getline(ss, line)) {
                //(1) remove commas from line
                auto noSpaceEnd = remove(line.begin(), line.end(), ',');
                if (noSpaceEnd == line.end()) { // output trace does not have comma so something went wrong with the command
                    std::cerr << "nvidia-smi call was incorrectly executed!" << std::endl;
                    continue;
                }
                line.erase(noSpaceEnd, line.end());

                //(2) read in element by element 
                std::istringstream sss(line);
                std::string gpuIdx;
                sss >> gpuIdx; 
                size_t idx = static_cast<size_t>(std::stoull(gpuIdx));
                if (idx >= nGPUs_) {
                    std::cerr << "GPU index out of range: " << idx << std::endl;
                    continue;  // Handle the error appropriately
                }
                for (auto const & d : data) {
                    std::string temp; 
                    sss >> temp; 
                    switch (d) {
                        case uprofile::NvidiaMonitor::Data::Usage: {
                            m_gpuUsage[idx] = !temp.empty() ? stof(temp) : 0.f;
                            break;  
                        };
                        case uprofile::NvidiaMonitor::Data::UsedMem: {
                            m_usedMem[idx] = !temp.empty() ? stoi(temp) * 1024 : 0;    // MiB to KiB
                            break;  
                        };
                        case uprofile::NvidiaMonitor::Data::TotalMem: {
                            m_totalMem[idx] = !temp.empty() ? stoi(temp) * 1024 : 0; // MiB to KiB
                            break;  
                        };
                    }
                }
            }
        } catch (const std::exception& err) {
            std::cerr << errorMsg << std::endl;
        }
    #endif
}

const std::vector<float>& uprofile::NvidiaMonitor::getUsage()
{
    update_gpu_data(std::vector<uprofile::NvidiaMonitor::Data>{
        uprofile::NvidiaMonitor::Data::Usage
    });
    return m_gpuUsage;
}

void uprofile::NvidiaMonitor::getMemory(std::vector<int>& usedMem, std::vector<int>& totalMem)
{
    update_gpu_data(std::vector<uprofile::NvidiaMonitor::Data>{
        uprofile::NvidiaMonitor::Data::UsedMem, 
        uprofile::NvidiaMonitor::Data::TotalMem
    });
    usedMem = m_usedMem;
    totalMem = m_totalMem;
}
