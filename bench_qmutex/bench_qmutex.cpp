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
#include <mutex>

namespace {
/**
 * This benchmark measures the performance of the *non-contended* overhead of
 * locking various mutex classes.
 *
 * For more information on QMutex, see:
 * http://woboq.com/blog/internals-of-qmutex-in-qt5.html
 *
 * TODO: add tests for the contented case as well.
 */
class BenchQMutex : public QObject
{
    Q_OBJECT

private slots:
    void benchQMutex_data()
    {
        QTest::addColumn<bool>("recursive");
        QTest::newRow("recursive") << true;
        QTest::newRow("non-recursive") << false;
    }

    void benchQMutex()
    {
        QFETCH(bool, recursive);
        QMutex mutex(recursive ? QMutex::Recursive : QMutex::NonRecursive);
        QBENCHMARK {
            QMutexLocker lock(&mutex);
            Q_UNUSED(lock);
        }
    }

    void benchStdMutex()
    {
        std::mutex mutex;
        QBENCHMARK {
            std::lock_guard<std::mutex> lock(mutex);
            Q_UNUSED(lock);
        }
    }

    void benchStdRecursiveMutex()
    {
        std::recursive_mutex mutex;
        QBENCHMARK {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            Q_UNUSED(lock);
        }
    }

    void benchQReadWriteLockRead_data()
    {
        benchQMutex_data();
    }

    void benchQReadWriteLockRead()
    {
        QFETCH(bool, recursive);
        QReadWriteLock mutex(recursive ? QReadWriteLock::Recursive : QReadWriteLock::NonRecursive);
        QBENCHMARK {
            QReadLocker lock(&mutex);
            Q_UNUSED(lock);
        }
    }

    void benchQReadWriteLockWrite_data()
    {
        benchQMutex_data();
    }

    void benchQReadWriteLockWrite()
    {
        QFETCH(bool, recursive);
        QReadWriteLock mutex(recursive ? QReadWriteLock::Recursive : QReadWriteLock::NonRecursive);
        QBENCHMARK {
            QWriteLocker lock(&mutex);
            Q_UNUSED(lock);
        }
    }
};
}

QTEST_GUILESS_MAIN(BenchQMutex)

#include "bench_qmutex.moc"
