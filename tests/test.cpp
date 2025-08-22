// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2025 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al

#include <catch2/catch_test_macros.hpp>
#include <unistd.h>
#include <uprofile.h>

static const std::string filename = "./test.log";

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.is_open();
}

int fileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.good()) return 0;
    return file.tellg();
}

TEST_CASE("Uprofile instant metrics", "[instant]")
{
    uprofile::start(filename.c_str());

    SECTION("Instant CPU Usage")
    {
        std::vector<float> loads = uprofile::getInstantCpuUsage();
        for (auto it = loads.cbegin(); it != loads.cend(); ++it) {
            REQUIRE(*it >= 0.);
            REQUIRE(*it <= 100.0);
        }
    }

    SECTION("No update in the file")
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        REQUIRE(file.good());
        REQUIRE(file.tellg() == 0);
        file.close();
        sleep(1);
        file.open(filename, std::ios::binary | std::ios::ate);
        REQUIRE(file.tellg() == 0);
    }

    uprofile::stop();
    std::remove(filename.c_str());
}

TEST_CASE("Uprofile monitoring metrics", "[monitoring]")
{
    uprofile::start(filename.c_str());
    uprofile::startCPUUsageMonitoring(200);
    SECTION("File existency")
    {
        REQUIRE( fileExists(filename) );
    }

    SECTION("File update")
    {
        // Check that the file grows over time
        auto size = fileSize(filename);
        sleep(1);
        REQUIRE(fileSize(filename) > size);
    }
    uprofile::stop();
    std::remove(filename.c_str());
}

TEST_CASE("Uprofile rotating files", "[rotation]")
{
    // The two rotating files will have a maximum of 100 bytes in total
    int maxSize = 100;  // bytes
    uprofile::start(filename.c_str(), maxSize);

    // Enable all monitoring to stress the library
    uprofile::startCPUUsageMonitoring(50);
    uprofile::startSystemMemoryMonitoring(50);
    uprofile::startProcessMemoryMonitoring(50);

    size_t pos = filename.rfind('.');
    std::string file1 = filename;
    std::string file2 = filename;
    file1.insert(pos,"_0");
    file2.insert(pos,"_1");

    SECTION("Two rotating files")
    {
        // Wait a couple of seconds to have several rotations
        sleep(2);
        REQUIRE( fileExists(file1) );
        REQUIRE( fileExists(file2) );
        
        // Check total size does not exceed the limit
        REQUIRE (fileSize(file1) + fileSize(file2) < maxSize);
    }

    uprofile::stop();
    std::remove(file1.c_str());
    std::remove(file2.c_str());
}