#-------------------------------------------------
#
# Project created by QtCreator 2016-08-07T08:48:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NeuralStyler
TEMPLATE = app


SOURCES += main.cpp\
        neuralstylerwindow.cpp \
    ffmpegprocess.cpp \
    paths.cpp \
    stylechainer.cpp \
    aboutdialog.cpp \
    qxtspanslider.cpp \
    styleintensitydialog.cpp \
    framedialog.cpp \
    mplayer.cpp \
    myprocess.cpp

HEADERS  += neuralstylerwindow.h \
    ffmpegprocess.h \
    paths.h \
    stylechainer.h \
    aboutdialog.h \
    WinVersion.h \
    qxtspanslider_p.h \
    qxtspanslider.h \
    styleintensitydialog.h \
    framedialog.h \
    mplayer.h \
    myprocess.h

FORMS    += neuralstylerwindow.ui \
    aboutdialog.ui \
    styleintensitydialog.ui \
    framedialog.ui

unix{

QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN/../lib'"
TARGET=neuralstyler

}

RESOURCES += \
    neuralstyler.qrc
RC_FILE = neuralstyler.rc
