QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets websockets network

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qrcode_win.cpp

HEADERS += \
    mainwindow.h \
    qrcode_win.h \
    qrencode.h

FORMS += \
    mainwindow.ui \
    qrcode_win.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/./ -lqrencode

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
