/**
 *
 * Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QtTest>
#include <QObject>
#include <QWidget>

#include <string>
#include <vector>

namespace {
const size_t NUM_ALLOCS = 1000;

template<typename T>
void benchAllocType()
{
    std::vector<T*> allocated(NUM_ALLOCS);
    QBENCHMARK {
        for (auto& p : allocated) {
            p = new T;
            __iteration_controller.next();
        }
        for (auto p : allocated) {
            delete p;
            __iteration_controller.next();
        }
    }
}

struct Foo
{
    QStringList strings() const
    { return m_strings; }
    QStringList m_strings;
};

struct Bar
{
    double a;
    double b;
};
struct BarMovable
{
    double a;
    double b;
};
}
Q_DECLARE_TYPEINFO(BarMovable, Q_MOVABLE_TYPE);
namespace {
class BenchQt : public QObject
{
    Q_OBJECT

private slots:
    void benchMallocFree_data()
    {
        QTest::addColumn<size_t>("size");
        for (size_t i = 1; i <= 12; ++i) {
            size_t size = 1 << i;
            QTest::newRow(std::to_string(size).data()) << size;
        }
    }
    void benchMallocFree()
    {
        QFETCH(size_t, size);
        std::vector<void*> allocated(NUM_ALLOCS);
        QBENCHMARK {
            for (auto& p : allocated) {
                p = malloc(size);
                __iteration_controller.next();
            }
            for (auto p : allocated) {
                free(p);
                __iteration_controller.next();
            }
        }
    }
    // calls malloc with randomized sizes
    void benchMallocFreeRand()
    {
        std::vector<void*> allocated(NUM_ALLOCS);
        std::vector<size_t> sizes(NUM_ALLOCS);
        // this distribution has it's mean value at 128, but assumes the tails
        // on both sides are equally common which is not the case.
        // TODO: use a different distribution to favor small allocations
        std::generate(sizes.begin(), sizes.end(), [] { return 1 << (std::rand() % 12 + 1); });
        QBENCHMARK {
            for (size_t i = 0; i < NUM_ALLOCS; ++i) {
                allocated[i] = malloc(sizes[i]);
                __iteration_controller.next();
            }
            for (auto p : allocated) {
                free(p);
                __iteration_controller.next();
            }
        }
    }
    void benchAllocQObject()
    {
        benchAllocType<QObject>();
    }
    void benchAllocQWidget()
    {
        benchAllocType<QWidget>();
    }

    void benchQStringCompareRaw()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == "foo");
            Q_UNUSED(equal);
        }
    }

    void benchQStringCompareLatin1()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == QLatin1String("foo"));
            Q_UNUSED(equal);
        }
    }

    void benchQStringCompareLiteral()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == QStringLiteral("foo"));
            Q_UNUSED(equal);
        }
    }

    void benchQStringConcatSlow()
    {
        const QString foo = QStringLiteral("foo");
        const QString bar = QStringLiteral("foo");
        const QString asdf = QStringLiteral("asdf");
        QBENCHMARK {
            QString concat = foo;
            concat += bar;
            concat += asdf;
            Q_UNUSED(concat);
        }
    }

    void benchQStringConcatFast()
    {
        const QString foo = QStringLiteral("foo");
        const QString bar = QStringLiteral("foo");
        const QString asdf = QStringLiteral("asdf");
        QBENCHMARK {
            QString concat = foo % bar % asdf;
            Q_UNUSED(concat);
        }
    }

    void benchQStringArgSlow()
    {
        const QString foo = QStringLiteral("foo");
        const QString bar = QStringLiteral("bar");
        const QString asdf = QStringLiteral("asdf");
        const QString tpl = QStringLiteral("foo: %1, bar: %2, asdf: %3");
        QBENCHMARK {
            QString concat =  tpl.arg(foo).arg(bar).arg(asdf);
            Q_UNUSED(concat);
        }
    }

    void benchQStringArgFast()
    {
        const QString foo = QStringLiteral("foo");
        const QString bar = QStringLiteral("bar");
        const QString asdf = QStringLiteral("asdf");
        const QString tpl = QStringLiteral("foo: %1, bar: %2, asdf: %3");
        QBENCHMARK {
            QString concat =  tpl.arg(foo, bar, asdf);
            Q_UNUSED(concat);
        }
    }

    void benchQStringMid()
    {
        const QString needle = QStringLiteral("foo");
        const QString haystack = needle.repeated(10);
        QBENCHMARK {
            bool equal = haystack.mid(needle.length(), needle.length()) == needle;
            Q_UNUSED(equal);
        }
    }

    void benchQStringMidRef()
    {
        const QString needle = QStringLiteral("foo");
        const QString haystack = needle.repeated(10);
        QBENCHMARK {
            bool equal = haystack.midRef(needle.length(), needle.length()) == needle;
            Q_UNUSED(equal);
        }
    }

    void benchQStringListLast()
    {
        Foo foo;
        for (size_t i = 0; i < NUM_ALLOCS; ++i) {
            foo.m_strings << QString::number(i);
        }
        const QString needle = foo.m_strings.last();
        QBENCHMARK {
            bool equal = foo.strings().last() == needle;
            Q_UNUSED(equal);
        }
    }

    void benchQStringListConstLast()
    {
        Foo foo;
        for (size_t i = 0; i < NUM_ALLOCS; ++i) {
            foo.m_strings << QString::number(i);
        }
        const QString needle = foo.m_strings.last();
        QBENCHMARK {
            const auto& list = foo.strings();
            bool equal = list.last() == needle;
            Q_UNUSED(equal);
        }
    }

    void benchQList()
    {
        QBENCHMARK {
            QList<Bar> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(Bar());
            }
        }
    }
    void benchQVector()
    {
        QBENCHMARK {
            QVector<Bar> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(Bar());
            }
        }
    }
    void benchQVectorMovable()
    {
        QBENCHMARK {
            QVector<BarMovable> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(BarMovable());
            }
        }
    }

    void benchCurrentDateTime()
    {
        QBENCHMARK {
            QDateTime::currentDateTime();
        }
    }
    void benchCurrentDateTimeUtc()
    {
        QBENCHMARK {
            QDateTime::currentDateTimeUtc();
        }
    }
    void benchQElapsedTimer()
    {
        QBENCHMARK {
            QElapsedTimer t;
            t.start();
        }
    }

    void benchMutex_data()
    {
        QTest::addColumn<bool>("recursive");
        QTest::newRow("recursive") << true;
        QTest::newRow("non-recursive") << false;
    }
    void benchMutex()
    {
        QFETCH(bool, recursive);
        QMutex mutex(recursive ? QMutex::Recursive : QMutex::NonRecursive);
        QBENCHMARK {
            QMutexLocker lock(&mutex);
        }
    }
    void benchReadWriteLockRead_data()
    {
        benchMutex_data();
    }
    void benchReadWriteLockRead()
    {
        QFETCH(bool, recursive);
        QReadWriteLock mutex(recursive ? QReadWriteLock::Recursive : QReadWriteLock::NonRecursive);
        QBENCHMARK {
            QReadLocker lock(&mutex);
        }
    }
    void benchReadWriteLockWrite_data()
    {
        benchMutex_data();
    }
    void benchReadWriteLockWrite()
    {
        QFETCH(bool, recursive);
        QReadWriteLock mutex(recursive ? QReadWriteLock::Recursive : QReadWriteLock::NonRecursive);
        QBENCHMARK {
            QWriteLocker lock(&mutex);
        }
    }
};
}

QTEST_MAIN(BenchQt)

#include "bench_qt.moc"
