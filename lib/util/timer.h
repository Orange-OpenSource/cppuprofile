// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#ifndef TIMER_H_
#define TIMER_H_

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

using namespace std;

class Timer
{
public:
    explicit Timer(int interval /* ms */ = 0);
    virtual ~Timer();

    void setTimeout(const function<void(void)>& timeout);
    void setInterval(int interval);
    void start();
    void stop();
    bool isRunning() const;

private:
    unique_ptr<thread> m_th;
    bool m_running;
    int m_interval;
    mutable std::mutex m_mutex;
    std::function<void(void)> m_timeout;
};

#endif // TIMER_H_
