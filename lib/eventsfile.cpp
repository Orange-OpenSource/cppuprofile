// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2024 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "eventsfile.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace uprofile
{

EventsFile::EventsFile(const char* filepath, unsigned long long maxCapSize) :
    m_maxCapSize(maxCapSize)
{
    if (m_maxCapSize > 0) {
        string str = string(filepath);
        auto found = str.find_last_of('.');
        bool hasExtension = found != std::string::npos;
        string basePath = hasExtension ? str.substr(0, found) : str;
        string extension = hasExtension ? str.substr(found) : "";
        for (int i = 0; i < ROTATING_FILES_NUMBER; ++i) {
            m_filePaths.emplace_back(basePath + "_" + std::to_string(i) + extension);
        }
    } else {
        m_filePaths.emplace_back(filepath);
    }

    m_file.open(m_filePaths[m_currentFileIdx], std::ios::out);
    if (!m_file.is_open()) {
        std::cerr << "Failed to open file: " << m_filePaths[m_currentFileIdx] << std::endl;
    }
}

EventsFile::~EventsFile()
{
    m_file.close();
}

void EventsFile::write(const std::string& event, unsigned long long timestamp, const std::list<std::string>& data)
{
    string csvSeparator(";");
    stringstream ss;
    ss << event << csvSeparator << timestamp;
    for (auto it = data.cbegin(); it != data.cend(); ++it) {
        ss << csvSeparator << *it;
    }
    ss << "\n";
    string line = ss.str();

    // Total size of all rotating files should never exceed the defined max cap size
    if (m_maxCapSize > 0 && m_currentFileSize + line.size() > m_maxCapSize / ROTATING_FILES_NUMBER) {
        rotateFile();
    }

    std::lock_guard<std::mutex> guard(m_fileMutex);
    m_file << line;
    m_file.flush();
    m_currentFileSize += line.size();
}

void EventsFile::rotateFile()
{
    m_file.close();
    m_currentFileSize = 0;
    if (++m_currentFileIdx >= m_filePaths.size()) {
        m_currentFileIdx = 0;
    }
    m_file.open(m_filePaths[m_currentFileIdx], std::ios::out);
    if (!m_file.is_open()) {
        std::cerr << "Failed to open file: " << m_filePaths[m_currentFileIdx] << std::endl;
    }
}

}
