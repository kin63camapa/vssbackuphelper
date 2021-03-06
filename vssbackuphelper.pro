QMAKE_CXXFLAGS_RELEASE += /MT
contains(QMAKE_TARGET.arch, x86_64):{
TARGET = ../../vssbackuphelper64
RC_FILE = vssbackuphelper64.rc
} else {
TARGET = ../../vssbackuphelper32
RC_FILE = vssbackuphelper32.rc
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    log.cpp \
    options.cpp \
    rawout.cpp

HEADERS += \
    log.h \
    options.h \
    rawout.h
OTHER_FILES += vssbackuphelper32.rc \
    vssbackuphelper64.rc \
    README.md

