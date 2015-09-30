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

namespace {
/**
 * A benchmark for common QDir operations.
 *
 * Note that the tests should be compared between each other.
 * Individually, their absolute numbers don't have much meaning as they
 * depend on the number of files in your home dir, on whether you are using
 * an SSD, on your file system, etc. pp.
 *
 * But all the tests here do essentially the same, thus pick the faster pattern.
 *
 * A note on caches: the QBENCHMARK macro will repeat its context multiple time,
 * after the first iterations the file system caches will be hot and any difference
 * therein can be neglected as it drowns in the noise when we average over all
 * runs.
 */
class BenchQDir : public QObject
{
    Q_OBJECT

private slots:
    void benchQDirEntryList()
    {
        QDir dir = QDir::home();
        QBENCHMARK {
            int files = 0;
            int folders = 0;
            QVector<QString> entries;
            entries.reserve(1024);

            foreach (const QString& entry, dir.entryList()) {
                QFileInfo info(dir.filePath(entry));
                folders += info.isDir();
                files += info.isFile();
                entries << entry;
            }

            QCOMPARE(entries.size(), files + folders);
        }
    }

    void benchQDirEntryInfoList()
    {
        QDir dir = QDir::home();
        QBENCHMARK {
            int files = 0;
            int folders = 0;
            QVector<QString> entries;
            entries.reserve(1024);

            foreach (const QFileInfo& info, dir.entryInfoList()) {
                folders += info.isDir();
                files += info.isFile();
                entries << info.fileName();
            }

            QCOMPARE(entries.size(), files + folders);
        }
    }

    void benchQDirIterator()
    {
        QDir dir = QDir::home();
        QBENCHMARK {
            int files = 0;
            int folders = 0;
            QVector<QString> entries;
            entries.reserve(1024);

            QDirIterator it(dir);
            while (it.hasNext()) {
                it.next();
                const QFileInfo& info = it.fileInfo();
                folders += info.isDir();
                files += info.isFile();
                entries << info.fileName();
            }

            QCOMPARE(entries.size(), files + folders);
        }
    }
};
}

QTEST_GUILESS_MAIN(BenchQDir)

#include "bench_qdir.moc"
