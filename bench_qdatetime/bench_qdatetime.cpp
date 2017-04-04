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
#include <QDateTime>
#include <chrono>

#include "../util.h"

class BenchQDateTime : public QObject
{
    Q_OBJECT

private slots:
    Q_NEVER_INLINE void benchCurrentDateTime()
    {
        QBENCHMARK {
            auto a = QDateTime::currentDateTime();
            escape(&a);
            auto b = QDateTime::currentDateTime();
            auto diff = a.msecsTo(b);
            escape(&diff);
        }
    }

    Q_NEVER_INLINE void benchCurrentDateTimeUtc()
    {
        QBENCHMARK {
            auto a = QDateTime::currentDateTimeUtc();
            clobber();
            auto b = QDateTime::currentDateTimeUtc();
            auto diff = a.msecsTo(b);
            escape(&diff);
        }
    }

    Q_NEVER_INLINE void benchQElapsedTimer()
    {
        QBENCHMARK {
            QElapsedTimer t;
            t.start();
            escape(&t);
            auto diff = t.elapsed();
            escape(&diff);
        }
    }

    Q_NEVER_INLINE void benchChronoSystemClock()
    {
        QBENCHMARK {
            auto a = std::chrono::system_clock::now();
            escape(&a);
            auto b = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::microseconds>(b - a);
            escape(&diff);
        }
    }

    Q_NEVER_INLINE void benchChronoHighResolutionClock()
    {
        QBENCHMARK {
            auto a = std::chrono::high_resolution_clock::now();
            escape(&a);
            auto b = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::microseconds>(b - a);
            escape(&diff);
        }
    }

    Q_NEVER_INLINE void benchChronoSteadyClock()
    {
        QBENCHMARK {
            auto a = std::chrono::steady_clock::now();
            escape(&a);
            auto b = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::microseconds>(b - a);
            escape(&diff);
        }
    }
};

QTEST_GUILESS_MAIN(BenchQDateTime)

#include "bench_qdatetime.moc"
