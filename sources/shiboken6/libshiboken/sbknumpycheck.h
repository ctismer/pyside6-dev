/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SBKNUMPYCHECK_H
#define SBKNUMPYCHECK_H

#include <sbkpython.h>
#include <shibokenmacros.h>


// This header provides a PyArray_Check() definition that can be used to avoid
// having to include the numpy headers. When using numpy headers, make sure
// to include this header after them to skip the definition. Also remember
// that import_array() must then be called to initialize numpy.

namespace Shiboken::Numpy
{

/// Check whether the object is a PyArrayObject
/// \param pyIn object
/// \return Whether it is a PyArrayObject
LIBSHIBOKEN_API bool check(PyObject *pyIn);

} //namespace Shiboken::Numpy

#ifndef PyArray_Check
#  define PyArray_Check(op) Shiboken::Numpy::check(op)
#endif

#endif // SBKNUMPYCHECK_H
