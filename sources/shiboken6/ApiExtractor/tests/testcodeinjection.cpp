/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "testcodeinjection.h"
#include "testutil.h"
#include <abstractmetalang.h>
#include <modifications.h>
#include <textstream.h>
#include <typesystem.h>

#include <qtcompat.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtTest/QTest>

using namespace Qt::StringLiterals;

void TestCodeInjections::testReadFile_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<QString>("snippet");
    QTest::addColumn<QString>("expected");

    QTest::newRow("utf8")
        << QString::fromLatin1(":/utf8code.txt")
        << QString()
        << QString::fromUtf8("\xC3\xA1\xC3\xA9\xC3\xAD\xC3\xB3\xC3\xBA");

    QTest::newRow("snippet")
        << QString::fromLatin1(":/injectedcode.txt")
        << QString::fromLatin1("label")
        << QString::fromLatin1("code line");
}

void TestCodeInjections::testReadFile()
{
    QFETCH(QString, filePath);
    QFETCH(QString, snippet);
    QFETCH(QString, expected);

    const char* cppCode ="struct A {};\n";
    int argc = 0;
    char *argv[] = {nullptr};
    QCoreApplication app(argc, argv);

    QString attribute = u"file='"_s + filePath + u'\'';
    if (!snippet.isEmpty())
        attribute += u" snippet='"_s + snippet + u'\'';

    QString xmlCode = u"\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <conversion-rule class='target' "_s + attribute + u"/>\n\
            <inject-code class='target' "_s + attribute + u"/>\n\
            <value-type name='B'/>\n\
        </value-type>\n\
    </typesystem>\n"_s;
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode.toLocal8Bit().constData()));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, u"A");
    QCOMPARE(classA->typeEntry()->codeSnips().size(), 1);
    QString code = classA->typeEntry()->codeSnips().constFirst().code();
    QVERIFY(code.indexOf(expected) != -1);
    code = classA->typeEntry()->targetConversionRule();
    QVERIFY(code.indexOf(expected) != -1);
}

void TestCodeInjections::testInjectWithValidApiVersion()
{
    const char* cppCode ="struct A {};\n";
    const char* xmlCode = "\
    <typesystem package='Foo'>\n\
        <value-type name='A'>\n\
            <inject-code class='target' since='1.0'>\n\
                test Inject code\n\
            </inject-code>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, u"1.0"_s));
    QVERIFY(!builder.isNull());
    AbstractMetaClassList classes = builder->classes();
    AbstractMetaClass* classA = AbstractMetaClass::findClass(classes, u"A");
    QCOMPARE(classA->typeEntry()->codeSnips().size(), 1);
}

void TestCodeInjections::testInjectWithInvalidApiVersion()
{
    const char* cppCode ="struct A {};\n";
    const char* xmlCode  = "\
    <typesystem package=\"Foo\">\n\
        <value-type name='A'>\n\
            <inject-code class='target' since='1.0'>\n\
                test Inject code\n\
            </inject-code>\n\
        </value-type>\n\
    </typesystem>\n";

    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, u"0.1"_s));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, u"A");
    QCOMPARE(classA->typeEntry()->codeSnips().size(), 0);
}

void TestCodeInjections::testTextStream()
{
    StringStream str(TextStream::Language::Cpp);
    str << "void foo(int a, int b) {\n";
    {
        Indentation i(str);
        str << "if (a == b)\n" << indent << "return a;\n" << outdent
            << "#if Q_OS_WIN\nprint()\n#endif\nreturn a + b;\n";
    }
    str << "}\n\n// A table\n|"
        << AlignedField("bla", 40, QTextStream::AlignRight) << "|\n|"
        << AlignedField("bla", 40, QTextStream::AlignLeft) << "|\n|"
        << AlignedField(QString(), 40, QTextStream::AlignLeft) << "|\n";
    str << "\n2nd table\n|" << AlignedField("bla", 3, QTextStream::AlignLeft)
        << '|' << AlignedField(QString{}, 0, QTextStream::AlignLeft) << "|\n";

static const char expected[] = R"(void foo(int a, int b) {
    if (a == b)
        return a;
#if Q_OS_WIN
    print()
#endif
    return a + b;
}

// A table
|                                     bla|
|bla                                     |
|                                        |

2nd table
|bla||
)";

    QCOMPARE(str.toString(), QLatin1String(expected));
}

void TestCodeInjections::testTextStreamRst()
{
    // Test that sphinx error: "Inline strong start-string without end-string."
    // is avoided, that is, characters following a formatting end are escaped.

    StringStream str;
    str << rstBold << "QObject" << rstBoldOff << "'s properties..."
        << rstItalic << "some italic" << rstItalicOff << " followed by space.";

    static const char16_t expected[] =
        uR"(**QObject**\'s properties...*some italic* followed by space.)";

    QCOMPARE(str.toString(), expected);
}

QTEST_APPLESS_MAIN(TestCodeInjections)
