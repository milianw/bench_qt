TEMPLATE = app

QT += testlib core_private
CONFIG += c++11 testcase release

LIBS += -licuuc

linux|mac {
    QMAKE_CXXFLAGS += -g
}

SOURCES = bench_qstring.cpp
