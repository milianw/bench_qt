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
#include <QString>
#include "../util.h"

#include <codecvt>
#include <unicode/ucnv.h>

template<int MAX_SIZE>
class ConvertQStringToUtf8
{
public:
  const char* operator() (const QString& string)
  {
    std::codecvt_utf8_utf16<char16_t> codec;
    std::mbstate_t state;
    const char16_t* from_next = nullptr;
    char* to_next = nullptr;
    codec.out(state,
              reinterpret_cast<const char16_t*>(string.utf16()), reinterpret_cast<const char16_t*>(string.utf16() + string.size()), from_next,
              std::begin(m_buffer), std::end(m_buffer) - 1, to_next);
    return m_buffer;
  }

private:
  char m_buffer[MAX_SIZE] = {0};
};

// like qUtf8Printable, but faster yet limited to a certain maximum size
#define qFastUtf8Printable(max_size, string) ConvertQStringToUtf8<max_size>()(string)

template<int MAX_SIZE>
class ConvertQStringToUtf8_2
{
public:
  const char* operator() (const QString& string)
  {
    UErrorCode status = U_ZERO_ERROR;
    static UConverter *converter = ucnv_open("UTF-8", &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "failed to get utf8 converter (%d: %s)\n", status, u_errorName(status));
        return m_buffer;
    }
    ucnv_fromUChars(converter, m_buffer, MAX_SIZE - 1, string.utf16(), string.size(), &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "failed to convert string to utf8 (%d: %s)\n", status, u_errorName(status));
    }
    return m_buffer;
  }

private:
  char m_buffer[MAX_SIZE] = {0};
};

#define qFastUtf8Printable2(max_size, string) ConvertQStringToUtf8_2<max_size>()(string)

class BenchQString : public QObject
{
    Q_OBJECT

private slots:
    Q_NEVER_INLINE void benchQStringCompareRaw()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == "foo");
            Q_UNUSED(equal);
        }
    }

    Q_NEVER_INLINE void benchQStringCompareLatin1()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == QLatin1String("foo"));
            Q_UNUSED(equal);
        }
    }

    Q_NEVER_INLINE void benchQStringCompareLiteral()
    {
        const QString foo = QStringLiteral("foo");
        QBENCHMARK {
            bool equal = (foo == QStringLiteral("foo"));
            Q_UNUSED(equal);
        }
    }

    Q_NEVER_INLINE void benchQStringConcatSlow()
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

    Q_NEVER_INLINE void benchQStringConcatFast()
    {
        const QString foo = QStringLiteral("foo");
        const QString bar = QStringLiteral("foo");
        const QString asdf = QStringLiteral("asdf");
        QBENCHMARK {
            QString concat = foo % bar % asdf;
            Q_UNUSED(concat);
        }
    }

    Q_NEVER_INLINE void benchQStringArgSlow()
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

    Q_NEVER_INLINE void benchQStringArgFast()
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

    Q_NEVER_INLINE void benchQStringMid()
    {
        const QString needle = QStringLiteral("foo");
        const QString haystack = needle.repeated(10);
        QBENCHMARK {
            bool equal = haystack.mid(needle.length(), needle.length()) == needle;
            Q_UNUSED(equal);
        }
    }

    Q_NEVER_INLINE void benchQStringMidRef()
    {
        const QString needle = QStringLiteral("foo");
        const QString haystack = needle.repeated(10);
        QBENCHMARK {
            bool equal = haystack.midRef(needle.length(), needle.length()) == needle;
            Q_UNUSED(equal);
        }
    }

    Q_NEVER_INLINE void benchQPrintable()
    {
        const QString string = QStringLiteral("123456789012345678901234567890");
        QBENCHMARK {
            escape(qPrintable(string));
        }
    }

    Q_NEVER_INLINE void benchQUtf8Printable()
    {
        const QString string = QStringLiteral("123456789012345678901234567890");
        QBENCHMARK {
            escape(qUtf8Printable(string));
        }
    }

    Q_NEVER_INLINE void benchQFastUtf8Printable()
    {
        const QString string = QStringLiteral("123456789012345678901234567890");
        QBENCHMARK {
            escape(qFastUtf8Printable(64, string));
        }
    }

    Q_NEVER_INLINE void benchQFastUtf8Printable2()
    {
        const QString string = QStringLiteral("123456789012345678901234567890");
        QBENCHMARK {
            escape(qFastUtf8Printable2(64, string));
        }
    }
};

QTEST_GUILESS_MAIN(BenchQString)

#include "bench_qstring.moc"
