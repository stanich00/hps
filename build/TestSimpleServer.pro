#-------------------------------------------------
#
# Project created by QtCreator 2012-08-18T07:14:15
#
#-------------------------------------------------


TARGET = TestSimpleServer
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++0x

TEMPLATE = app


SOURCES += \
    ../src/HpsLocker.cpp \
    ../src/HpsMutex.cpp \
    ../src/HpsCommon.cpp \
    ../src/HpsLog.cpp \
    ../src/HpsProgArgs.cpp \
    ../src/HpsConfig.cpp \
    ../src/HpsTcpServer.cpp \
    ../src/main.cpp

HEADERS += \
    ../src/HpsLog.h \
    ../src/HpsLocker.h \
    ../src/HpsMutex.h \
    ../src/HpsCommon.h \
    ../src/HpsStdOutLogWriter.h \
    ../src/HpsLogWriter.h \
    ../src/HpsProgArgs.h \
    ../src/HpsConfig.h \
    ../src/HpsTcpServer.h
