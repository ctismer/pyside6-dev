// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "spanuser.h"

#include <numeric>

SpanUser::SpanUser() = default;

bool SpanUser::enabled()
{
#if __cplusplus >= 202002L
    return true;
#else
    return false;
#endif
}

#if __cplusplus >= 202002L
IntSpan3 SpanUser::getIntSpan3()
{
    static int iv[] = {1, 2, 3};
    return IntSpan3(iv);
}

ConstIntSpan3 SpanUser::getConstIntSpan3()
{
    static const int civ[] = {1, 2, 3};
    return ConstIntSpan3(civ);
}

int SpanUser::sumIntSpan3(IntSpan3 isp3)
{
     return std::accumulate(isp3.begin(), isp3.end(), 0);
}

int SpanUser::sumConstIntSpan3(ConstIntSpan3 ispc3)
{
    return std::accumulate(ispc3.begin(), ispc3.end(), 0);
}
#endif // C++ 20