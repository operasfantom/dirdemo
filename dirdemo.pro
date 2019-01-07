TEMPLATE = subdirs
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SUBDIRS = src tests \
    tests
#src.file = src/src.pro
#CONFIG += ordered
