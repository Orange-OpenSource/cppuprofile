// Software Name : cppuprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD-3-Clause
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

#include "timer.h"

Timer::Timer(int interval) :
    m_interval(interval)
{
}

Timer::~Timer()
{
    stop();
}

void Timer::setInterval(int interval)
{
    m_interval = interval;
}

void Timer::setTimeout(const std::function<void(void)>& timeout)
{
    m_timeout = timeout;
}

bool Timer::isRunning() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_running;
}

void Timer::start()
{
    if (!m_th && m_interval > 0) {
        m_running = true;
        m_th = unique_ptr<thread>(new thread([=]() {
            while (isRunning()) {
                this_thread::sleep_for(chrono::milliseconds(m_interval));
                if (m_timeout) {
                    m_timeout();
                }
            }
        }));
    }
}

void Timer::stop()
{
    unique_lock<mutex> lk(m_mutex);
    m_running = false;
    lk.unlock();
    if (m_th) {
        m_th->join();
        m_th.reset();
    }
}
