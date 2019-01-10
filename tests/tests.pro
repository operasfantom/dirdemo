QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  \
        tst_test.cpp \
        ../src/directory_controller.cpp

HEADERS += ../src/directory_controller.h \
           tst_test.h
