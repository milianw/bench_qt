TEMPLATE = app

QT += testlib
CONFIG += c++11 testcase release

linux|mac {
    QMAKE_CXXFLAGS += -g
}

SOURCES = bench_qdatetime.cpp
