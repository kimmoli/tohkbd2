TARGET = harbour-dconfreaderwriter

CONFIG += link_pkgconfig
PKGCONFIG += mlite5

QT += dbus
QT -= gui

# DBus
system(qdbusxml2cpp config/com.kimmoli.dconfreaderwriter.xml -i readerwriter.h -a adaptor)

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
    config/$${TARGET}.service \
    config/com.kimmoli.dconfreaderwriter.xml

HEADERS += \
    src/readerwriter.h
