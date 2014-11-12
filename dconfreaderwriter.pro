TARGET = harbour-dconfreaderwriter

CONFIG += link_pkgconfig
PKGCONFIG += mlite5

QT += dbus
QT -= gui

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

systemd.path = /etc/systemd/user/
systemd.files = config/$${TARGET}.service

INSTALLS += target systemd

message($${DEFINES})

SOURCES += \
    src/dconfreaderwriter.cpp \
    src/readerwriter.cpp

OTHER_FILES += \
    rpm/$${TARGET}.spec \
    config/$${TARGET}.service

HEADERS += \
    src/readerwriter.h
