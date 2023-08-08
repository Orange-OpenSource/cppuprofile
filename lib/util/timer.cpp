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
    m_th(NULL),
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

bool Timer::isRunning()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_running;
}

void Timer::start()
{
    if (m_interval > 0 && m_th == NULL) {
        m_running = true;
        m_th = new thread([=]() {
            while (isRunning()) {
                this_thread::sleep_for(chrono::milliseconds(m_interval));
                if (m_timeout) {
                    m_timeout();
                }
            }
        });
    }
}

void Timer::stop()
{
    m_mutex.lock();
    m_running = false;
    m_mutex.unlock();
    if (m_th) {
        m_th->join();
        delete m_th;
        m_th = NULL;
    }
}
