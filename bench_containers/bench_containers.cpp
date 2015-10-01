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

#include <string>
#include <vector>
#include <algorithm>

namespace {
const size_t NUM_ALLOCS = 10000;
struct Foo
{
    QStringList strings() const
    { return m_strings; }
    QStringList m_strings;
};

struct Bar
{
    QString a;
    double b;
};
struct BarMovable
{
    QString a;
    double b;
};
}

Q_DECLARE_TYPEINFO(BarMovable, Q_MOVABLE_TYPE);

/**
 * A benchmark for various common container patterns.
 *
 * NOTE: The lists are not reserved up front for a reason.
 *       Only without it you can see a difference between
 *       the movable and non-movable QVector case.
 *
 * For a more in-depth comparison, see:
 * https://marcmutz.wordpress.com/effective-qt/containers/
 */
class BenchContainers : public QObject
{
    Q_OBJECT

private slots:
    // bar is too large and every node be allocated on the heap
    Q_NEVER_INLINE void benchQList()
    {
        const Bar item = Bar{QStringLiteral("foo bar asdf"), 42.0};
        QBENCHMARK {
            QList<Bar> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(item);
            }
        }
    }

    // nice consecutive memory, very fast
    Q_NEVER_INLINE void benchQVector()
    {
        const Bar item = Bar{QStringLiteral("foo bar asdf"), 42.0};
        QBENCHMARK {
            QVector<Bar> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(item);
            }
        }
    }

    // nice consecutive memory, and simple memmove when reallocating, thus faster
    Q_NEVER_INLINE void benchQVectorMovable()
    {
        const BarMovable item = BarMovable{QStringLiteral("foo bar asdf"), 42.0};
        QBENCHMARK {
            QVector<BarMovable> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.append(item);
            }
        }
    }

    // also fast and can leverage move semantics internally
    Q_NEVER_INLINE void benchVector()
    {
        const Bar item = Bar{QStringLiteral("foo bar asdf"), 42.0};
        QBENCHMARK {
            std::vector<Bar> list;
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                list.push_back(item);
            }
        }
    }

    Q_NEVER_INLINE void benchQHashIndex()
    {
        QHash<size_t, size_t> map;
        map.reserve(NUM_ALLOCS);
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i;
        }
        auto keys = map.keys();
        std::random_shuffle(keys.begin(), keys.end());
        QBENCHMARK {
            foreach(auto key, keys) {
                auto value = map[key];
                Q_UNUSED(value);
            }
        }
    }

    Q_NEVER_INLINE void benchQMapIndex()
    {
        QMap<size_t, size_t> map;
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i;
        }
        auto keys = map.keys();
        std::random_shuffle(keys.begin(), keys.end());
        QBENCHMARK {
            foreach(auto key, keys) {
                auto value = map[key];
                Q_UNUSED(value);
            }
        }
    }

    Q_NEVER_INLINE void benchQHashForeachNaive()
    {
        QHash<size_t, size_t> map;
        map.reserve(NUM_ALLOCS);
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i;
        }
        QBENCHMARK {
            foreach(auto key, map.keys()) {
                auto value = map[key];
                Q_UNUSED(value);
                Q_UNUSED(key);
            }
        }
    }

    Q_NEVER_INLINE void benchQHashForeach()
    {
        QHash<size_t, size_t> map;
        map.reserve(NUM_ALLOCS);
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i;
        }
        QBENCHMARK {
            for(auto it = map.begin(), end = map.end(); it != end; ++it) {
                auto key = it.key();
                auto value = it.value();
                Q_UNUSED(value);
                Q_UNUSED(key);
            }
        }
    }

    Q_NEVER_INLINE void benchQMapForeach()
    {
        QMap<size_t, size_t> map;
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i;
        }
        QBENCHMARK {
            for(auto it = map.begin(), end = map.end(); it != end; ++it) {
                auto key = it.key();
                auto value = it.value();
                Q_UNUSED(value);
                Q_UNUSED(key);
            }
        }
    }

    Q_NEVER_INLINE void benchQHashContainsLookup()
    {
        QHash<size_t, size_t> map;
        map.reserve(NUM_ALLOCS);
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i + 1; // assume zero is invalid
        }
        QBENCHMARK {
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                size_t value = 0;
                if (map.contains(i)) {
                    value = map[i];
                }
                Q_UNUSED(value);
            }
        }
    }

    Q_NEVER_INLINE void benchQHashValueLookup()
    {
        QHash<size_t, size_t> map;
        map.reserve(NUM_ALLOCS);
        for(size_t i = 0; i < NUM_ALLOCS; ++i) {
            map[i] = i + 1; // assume zero is invalid
        }
        QBENCHMARK {
            for(size_t i = 0; i < NUM_ALLOCS; ++i) {
                size_t value = map.value(i, 0);
                Q_UNUSED(value);
            }
        }
    }
};

QTEST_GUILESS_MAIN(BenchContainers)

#include "bench_containers.moc"
