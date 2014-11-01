TARGET = harbour-tohkbd2

QT += dbus
QT -= gui

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

systemd.path = /etc/systemd/system/
systemd.files = config/$${TARGET}.service

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/$${TARGET}.conf

ambience.path = /usr/share/ambience/$${TARGET}
ambience.files = ambience/$${TARGET}.ambience

images.path = $${ambience.path}/images
images.files = ambience/images/*

vkblayout.path = /usr/share/maliit/plugins/com/jolla/layouts/
vkblayout.files = config/$${TARGET}.conf config/$${TARGET}.qml

INSTALLS += target systemd udevrule dbusconf ambience images vkblayout

message($${DEFINES})

SOURCES += \
    src/tohkbd2-daemon.cpp \
    src/toh.cpp \
    src/tca8424.cpp \
    src/worker.cpp \
    src/tohkeyboard.cpp \
    src/uinputif.cpp

HEADERS += \
    src/toh.h \
    src/tca8424.h \
    src/worker.h \
    src/tohkeyboard.h \
    src/uinputif.h

OTHER_FILES += \
    rpm/$${TARGET}.spec \
    config/$${TARGET}.service \
    config/$${TARGET}.conf \
    config/$${TARGET}.qml
