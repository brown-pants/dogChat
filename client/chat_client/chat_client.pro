QT       += core gui network

TEMPLATE = app

TARGET = chat_client

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    util/include/ \
    storage/include/ \
    widgets/include/ \
    ../3rdparty/

SOURCES += \
    main.cpp \
    appinit.cpp \
    storage/src/storagemanager.cpp \
    util/src/util.cpp \
    widgets/src/loginwnd.cpp \
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
    storage/include/storagemanager.h \
    util/include/util.h \
    widgets/include/loginwnd.h \
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
    widgets/ui/loginwnd.ui \
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

CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
} else {
    BUILD_TYPE = release
}

# 3rdparty
win32 {
    LIBS += -L$$OUT_PWD/../3rdparty/pinyin4cpp/$$BUILD_TYPE -lpinyin4cpp
}
unix {
    LIBS += -L$$OUT_PWD/../3rdparty/pinyin4cpp -lpinyin4cpp
}
