// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "bucket.h"

#include <iostream>

#ifdef _WIN32 // _WIN32 is defined by all Windows 32 and 64 bit compilers, but not by others.
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  define SLEEP(x) Sleep(x)
#else
#  include <unistd.h>
#  define SLEEP(x) usleep(x)
#endif

void Bucket::push(int x)
{
#ifdef Py_NOGIL
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    m_data.push_back(x);
}

int Bucket::pop(void)
{
    int x = 0;

#ifdef Py_NOGIL
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    if (m_data.size() > 0) {
        x = m_data.front();
        m_data.pop_front();
    }

    return x;
}

bool Bucket::empty()
{
#ifdef Py_NOGIL
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    return m_data.empty();
}

void Bucket::lock()
{
    m_locked = true;
    while (m_locked) {
        SLEEP(300);
    }
}

void Bucket::unlock()
{
    m_locked = false;
}

bool Bucket::virtualBlockerMethod()
{
    lock();
    // The return value was added just for diversity sake.
    return true;
}
