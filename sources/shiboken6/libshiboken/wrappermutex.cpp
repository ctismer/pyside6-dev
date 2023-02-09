// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "wrappermutex.h"

namespace Shiboken {
namespace Wrapper {

#ifdef Py_GIL_DISABLED

WrapperMutexStore &mutexStore()
{
    static WrapperMutexStore instance;
    return instance;
}

std::recursive_mutex &WrapperMutexStore::get(PyObject *self)
{
    void *key = reinterpret_cast<void *>(self); // Pointer identity
    std::lock_guard<std::recursive_mutex> guard(m_mapMutex);
    return m_mutexMap[key]; // default-constructs if missing
}

#endif // Py_GIL_DISABLED

} // namespace Wrapper
} // namespace Shiboken
