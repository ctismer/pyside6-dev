// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef SHIBOKEN_WRAPPERMUTEX_H
#define SHIBOKEN_WRAPPERMUTEX_H

#include <mutex>
#include <unordered_map>
#include <Python.h>

namespace Shiboken {
namespace Wrapper {

#ifdef Py_GIL_DISABLED

// Thread-safe map to associate PyObject* instances with a mutex
class LIBSHIBOKEN_API WrapperMutexStore
{
public:
    std::recursive_mutex &get(PyObject *self);

private:
    std::unordered_map<void *, std::recursive_mutex> m_mutexMap;
    std::recursive_mutex m_mapMutex;
};

// Singleton accessor
LIBSHIBOKEN_API WrapperMutexStore &mutexStore();

LIBSHIBOKEN_API inline std::recursive_mutex &getMutex(PyObject *self)
{
    return mutexStore().get(self);
}

#endif // Py_GIL_DISABLED

} // namespace Wrapper
} // namespace Shiboken

#endif // SHIBOKEN_WRAPPERMUTEX_H
