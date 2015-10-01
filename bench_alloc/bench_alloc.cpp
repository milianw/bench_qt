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

#include "../util.h"

namespace {
const size_t NUM_ALLOCS = 1000;

template<typename T>
void benchAllocType()
{
    QBENCHMARK {
        for (size_t i = 0; i < NUM_ALLOCS; ++i) {
            T* p = new T;
            escape(p);
            delete p;
            clobber();
            __iteration_controller.next();
        }
    }
}
}

/**
 * Various benchmarks related to the system allocator.
 *
 * Note that all test cases bench both, allocation and deallocation.
 * The returned numbers should roughly correspond to the cost of an allocation
 * or deallocation. It is tough to benchmark only allocation or deallocation,
 * as then the benchmarks easily allocate too much memory and go OOM.
 *
 * Furthermore note that most of these tests are somewhat best-case benchmarks
 * for the allocator: No threads are contending for the heap, the allocator
 * stays code stays in cache, ...
 *
 * Still, the results are interesting nonetheless.
 */
class BenchAlloc : public QObject
{
    Q_OBJECT

private slots:
    Q_NEVER_INLINE void benchMallocFree_data()
    {
        QTest::addColumn<size_t>("size");
        for (size_t i = 1; i <= 12; ++i) {
            size_t size = 1 << i;
            QTest::newRow(std::to_string(size).data()) << size;
        }
    }

    // bench repeated malloc and free for various constant sizes
    Q_NEVER_INLINE void benchMallocFree()
    {
        QFETCH(size_t, size);

        QBENCHMARK {
            for (size_t i = 0; i < NUM_ALLOCS; ++i) {
                void* p = malloc(size);
                escape(p);
                free(p);
                clobber();
                __iteration_controller.next();
            }
        }
    }

    // bench repeated malloc and free with randomized sizes
    Q_NEVER_INLINE void benchMallocFreeRand()
    {
        std::vector<size_t> sizes(NUM_ALLOCS);
        // this distribution has it's mean value at 128, but assumes the tails
        // on both sides are equally common which is not the case.
        // TODO: use a different distribution to favor small allocations
        std::generate(sizes.begin(), sizes.end(), [] { return 1 << (std::rand() % 12 + 1); });
        QBENCHMARK {
            for (size_t i = 0; i < NUM_ALLOCS; ++i) {
                void* p = malloc(sizes[i]);
                escape(p);
                free(p);
                clobber();
                __iteration_controller.next();
            }
        }
    }

    // bench the repeated (de)allocation of QObject
    Q_NEVER_INLINE void benchAllocQObject()
    {
        benchAllocType<QObject>();
    }

    // bench the repeated (de)allocation of QObject
    Q_NEVER_INLINE void benchAllocQWidget()
    {
        benchAllocType<QWidget>();
    }

    Q_NEVER_INLINE void benchMemcpy_data()
    {
        benchMallocFree_data();
    }

    Q_NEVER_INLINE void benchMemcpy()
    {
        QFETCH(size_t, size);
        std::vector<char> target(size);
        std::vector<char> source(size);
        QBENCHMARK {
            memcpy(target.data(), source.data(), size);
            clobber();
        }
    }
};

QTEST_MAIN(BenchAlloc)

#include "bench_alloc.moc"
