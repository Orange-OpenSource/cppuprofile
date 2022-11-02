// Software Name : uprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "memoryusage.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

void uprofile::getSystemMemory(int &totalMem, int &availableMem, int &freeMem)
{
    ifstream meminfo("/proc/meminfo");
    // TODO: simplif because it always 3 first lines!
    //MemTotal: 515164 kB
    //MemFree: 7348 kB
    //MemAvailable: 7348 kB
    string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemTotal", 0) == 0)
        {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> totalMem;
        }
        else if (line.rfind("MemFree", 0) == 0)
        {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> freeMem;
        }
        else if (line.rfind("MemAvailable", 0) == 0)
        {
            stringstream ls(line);
            ls.ignore(256, ' ');
            ls >> availableMem;
        }
    }
}

void uprofile::getProcessMemory(int &rss, int &shared)
{
    int tSize = 0, resident = 0, share = 0;
    ifstream buffer("/proc/self/statm");
    buffer >> tSize >> resident >> share;
    buffer.close();

    long page_size_kb = getpagesize() / 1024;
    rss = resident * page_size_kb;
    shared = share * page_size_kb;
}