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

// PYSIDE-2221: This is only a demo from Sam Gross.
//              We need to automate such locking in the code generator.
// The patch stays here as a comment until automated.
void Bucket::push(int x)
{
#ifdef Py_GIL_DISABLED
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    m_data.push_back(x);
}

int Bucket::pop(void)
{
    int x = 0;

#ifdef Py_GIL_DISABLED
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    if (!m_data.empty()) {
        x = m_data.front();
        m_data.pop_front();
    }

    return x;
}

bool Bucket::empty()
{
#ifdef Py_GIL_DISABLED
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
