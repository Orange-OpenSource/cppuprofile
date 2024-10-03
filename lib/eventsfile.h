// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2024 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef EVENTSFILE_H_
#define EVENTSFILE_H_

#include <fstream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace uprofile
{

class EventsFile
{
public:
    EventsFile(const char* filepath, unsigned long long maxCapSize);
    ~EventsFile();

    void write(const std::string& event, unsigned long long timestamp, const std::list<std::string>& data);

    static const int ROTATING_FILES_NUMBER = 2;

private:
    std::mutex m_fileMutex;
    std::ofstream m_file;
    std::vector<std::string> m_filePaths;
    unsigned int m_currentFileIdx = 0;
    unsigned long long m_currentFileSize = 0;
    unsigned long long m_maxCapSize = 0;

    void rotateFile();
};
using EventsFilePtr = std::shared_ptr<EventsFile>;

}

#endif /* EVENTSFILE_H_ */
