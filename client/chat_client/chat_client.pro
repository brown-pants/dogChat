QT       += core gui network

TEMPLATE = app

TARGET = chat_client

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    util/include/ \
    widgets/include/ \
    ../3rdparty/

SOURCES += \
    main.cpp \
    appinit.cpp \
    util/src/util.cpp \
    widgets/src/friendlistitem.cpp \
    widgets/src/friendlistwidget.cpp \
    widgets/src/filemsgwidget.cpp \
    widgets/src/chooseemojiwidget.cpp \
    widgets/src/chatmsgitem.cpp \
    widgets/src/chatlistitem.cpp \
    widgets/src/chatwidget.cpp \
    widgets/src/controlwidget.cpp \
    widgets/src/chatlistwidget.cpp \
    widgets/src/mainwnd.cpp

HEADERS += \
    appinit.h \
    util/include/util.h \
    widgets/include/friendlistitem.h \
    widgets/include/friendlistwidget.h \
    widgets/include/filemsgwidget.h \
    widgets/include/chooseemojiwidget.h \
    widgets/include/chatmsgitem.h \
    widgets/include/chatlistitem.h \
    widgets/include/chatwidget.h \
    widgets/include/controlwidget.h \
    widgets/include/chatlistwidget.h \
    widgets/include/mainwnd.h

FORMS += \
    widgets/ui/friendlistitem.ui \
    widgets/ui/friendlistwidget.ui \
    widgets/ui/filemsgwidget.ui \
    widgets/ui/chooseemojiwidget.ui \
    widgets/ui/chatmsgitem.ui \
    widgets/ui/chatlistitem.ui \
    widgets/ui/chatwidget.ui \
    widgets/ui/controlwidget.ui \
    widgets/ui/chatlistwidget.ui \
    widgets/ui/mainwnd.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc

# BUILD_TYPE: [debug] [release]
CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
}
else {
    BUILD_TYPE = release
}

# BUILD_PATH = xxx\build\xxx
BUILD_PATH = $$clean_path($$OUT_PWD/..)

# 3rdparty
win32 {
    LIBS += $${BUILD_PATH}/3rdparty/pinyin4cpp/$${BUILD_TYPE}/libpinyin4cpp.a
} else {
    LIBS += $${BUILD_PATH}/3rdparty/pinyin4cpp/libpinyin4cpp.so
}
