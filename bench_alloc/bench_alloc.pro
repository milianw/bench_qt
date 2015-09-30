TEMPLATE = app

QT += testlib widgets
CONFIG += c++11 testcase release

linux|mac {
    QMAKE_CXXFLAGS += -g
}

SOURCES = bench_alloc.cpp