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

#include "testabstractmetatype.h"
#include "testutil.h"
#include <abstractmetafunction.h>
#include <abstractmetalang.h>
#include <typesystem.h>
#include <parser/codemodel.h>
#include <typeparser.h>

#include <qtcompat.h>

#include <QtTest/QTest>

using namespace Qt::StringLiterals;

void TestAbstractMetaType::parsing_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::newRow("primitive")
        << QString::fromLatin1("int") << QString::fromLatin1("int");
    QTest::newRow("ref")
        << QString::fromLatin1("int &") << QString::fromLatin1("int&");
    QTest::newRow("pointer")
        << QString::fromLatin1("int **") << QString::fromLatin1("int**");
    QTest::newRow("const ref")
        << QString::fromLatin1("const int &") << QString::fromLatin1("const int&");
    QTest::newRow("const pointer")
        << QString::fromLatin1("const int **") << QString::fromLatin1("const int**");
    QTest::newRow("const pointer const")
        << QString::fromLatin1("const int *const*") << QString::fromLatin1("const int*const*");
}

void TestAbstractMetaType::parsing()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QString errorMessage;
    const TypeInfo ti = TypeParser::parse(input, &errorMessage);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    const QString actual = ti.toString();
    QCOMPARE(actual, output);
}

void TestAbstractMetaType::testConstCharPtrType()
{
    const char* cppCode ="const char* justAtest();\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
        <primitive-type name='char'/>\n\
        <function signature='justAtest()' />\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());
    QCOMPARE(builder->globalFunctions().size(), 1);
    const auto func = builder->globalFunctions().constFirst();
    AbstractMetaType rtype = func->type();
    // Test properties of const char*
    QVERIFY(!rtype.isVoid());
    QCOMPARE(rtype.package(), u"Foo");
    QCOMPARE(rtype.name(), u"char");
    QVERIFY(rtype.isConstant());
    QVERIFY(!rtype.isArray());
    QVERIFY(!rtype.isContainer());
    QVERIFY(!rtype.isObject());
    QVERIFY(!rtype.isPrimitive()); // const char* differs from char, so it's not considered a primitive type by apiextractor
    QVERIFY(rtype.isNativePointer());
    QCOMPARE(rtype.referenceType(), NoReference);
    QVERIFY(!rtype.isValue());
    QVERIFY(!rtype.isValuePointer());
}

void TestAbstractMetaType::testApiVersionSupported()
{
    const char* cppCode ="class foo {}; class foo2 {};\n\
                          void justAtest(); void justAtest3();\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
        <value-type name='foo' since='0.1'/>\n\
        <value-type name='foo2' since='1.0'/>\n\
        <value-type name='foo3' since='1.1'/>\n\
        <function signature='justAtest()' since='0.1'/>\n\
        <function signature='justAtest2()' since='1.1'/>\n\
        <function signature='justAtest3()'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                false, u"1.0"_s));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 2);


    QCOMPARE(builder->globalFunctions().size(), 2);
}


void TestAbstractMetaType::testApiVersionNotSupported()
{
    const char* cppCode ="class object {};\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
        <value-type name='object' since='0.1'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode,
                                                                true, u"0.1"_s));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
}

void TestAbstractMetaType::testCharType()
{
    const char* cppCode ="char justAtest(); class A {};\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <primitive-type name='char'/>\n\
    <value-type name='A'/>\n\
    <function signature='justAtest()'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    QCOMPARE(classes.constFirst()->package(), u"Foo");

    const auto functions = builder->globalFunctions();
    QCOMPARE(functions.size(), 1);
    const auto func = functions.constFirst();
    AbstractMetaType rtype = func->type();
    // Test properties of const char*
    QVERIFY(!rtype.isVoid());
    QCOMPARE(rtype.package(), u"Foo");
    QCOMPARE(rtype.name(), u"char");
    QVERIFY(!rtype.isConstant());
    QVERIFY(!rtype.isArray());
    QVERIFY(!rtype.isContainer());
    QVERIFY(!rtype.isObject());
    QVERIFY(rtype.isPrimitive());
    QVERIFY(!rtype.isNativePointer());
    QCOMPARE(rtype.referenceType(), NoReference);
    QVERIFY(!rtype.isValue());
    QVERIFY(!rtype.isValuePointer());
}

void TestAbstractMetaType::testTypedef()
{
    const char* cppCode ="\
    struct A {\n\
        void someMethod();\n\
    };\n\
    typedef A B;\n\
    typedef B C;\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <value-type name='C' />\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    const AbstractMetaClass *c = AbstractMetaClass::findClass(classes, u"C");
    QVERIFY(c);
    QVERIFY(c->isTypeDef());
}

void TestAbstractMetaType::testTypedefWithTemplates()
{
    const char* cppCode ="\
    template<typename T>\n\
    class A {};\n\
    \n\
    class B {};\n\
    typedef A<B> C;\n\
    \n\
    void func(C c);\n";
    const char* xmlCode = "<typesystem package=\"Foo\">\n\
    <container-type name='A' type='list'/>\n\
    <value-type name='B' />\n\
    <function signature='func(A&lt;B&gt;)'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    const auto functions = builder->globalFunctions();
    QCOMPARE(functions.size(), 1);
    const auto function = functions.constFirst();
    AbstractMetaArgumentList args = function->arguments();
    QCOMPARE(args.size(), 1);
    const AbstractMetaArgument &arg = args.constFirst();
    AbstractMetaType metaType = arg.type();
    QCOMPARE(metaType.cppSignature(), u"A<B >");
}


void TestAbstractMetaType::testObjectTypeUsedAsValue()
{
    const char* cppCode ="\
    class A {\n\
        void method(A);\n\
    };\n";
    const char* xmlCode = "<typesystem package='Foo'>\n\
    <object-type name='A'/>\n\
    </typesystem>\n";
    QScopedPointer<AbstractMetaBuilder> builder(TestUtil::parse(cppCode, xmlCode));
    QVERIFY(!builder.isNull());

    AbstractMetaClassList classes = builder->classes();
    QCOMPARE(classes.size(), 1);
    const AbstractMetaClass *classA = AbstractMetaClass::findClass(classes, u"A");
    QVERIFY(classA);
    const auto overloads = classA->queryFunctionsByName(u"method"_s);
    QCOMPARE(overloads.size(), 1);
    const auto method = overloads.constFirst();
    QVERIFY(method);
    AbstractMetaArgumentList args = method->arguments();
    QCOMPARE(args.size(), 1);
    const AbstractMetaArgument &arg = args.constFirst();
    AbstractMetaType metaType = arg.type();
    QCOMPARE(metaType.cppSignature(), u"A");
    QVERIFY(metaType.isValue());
    QVERIFY(metaType.typeEntry()->isObject());
}

QTEST_APPLESS_MAIN(TestAbstractMetaType)
