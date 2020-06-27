#-------------------------------------------------
#
# Project created by QtCreator 2014-06-19T22:45:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = twitterdaemon
TEMPLATE = app


SOURCES += main.cpp\
    oauthfunctions.cpp \
    usergetpindialogue.cpp \
    twittermanager.cpp \
    twitteraccountdialogue.cpp \
    simplecrypt.cpp \
    stringconvert.cpp \
    filemanagement.cpp \
    keyvaluesnode.cpp \
    crypto.cpp \
    jsonwindow.cpp \
    simpletweet.cpp

HEADERS  += \
    oauthfunctions.h \
    stringconvert.h \
    usergetpindialogue.h \
    twittermanager.h \
    twitteraccountdialogue.h \
    simplecrypt.h \
    crypto.h \
    filemanagement.h \
    keyvaluesnode.h \
    jsonwindow.h \
    simpletweet.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/release/ -ltwitcurl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/debug/ -ltwitcurl
else:unix: LIBS += -L$$PWD/lib/ -ltwitcurl

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
