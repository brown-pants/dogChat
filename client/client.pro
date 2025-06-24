TEMPLATE = subdirs

SUBDIRS = \
    3rdparty/pinyin4cpp \
    chat_client

CONFIG += c++17

chat_client.depends = 3rdparty/pinyin4cpp
