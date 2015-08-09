TARGET = harbour-tohkbd2

QT += dbus
QT -= gui

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

vkblayout.path = /usr/share/maliit/plugins/com/jolla/layouts/
vkblayout.files = config/layouts/$${TARGET}.conf config/layouts/$${TARGET}.qml

INSTALLS += target udevrule vkblayout

message($${DEFINES})

SOURCES += \
    src/tohkbd2-daemon.cpp \
    src/toh.cpp \
    src/worker.cpp \
    src/tohkeyboard.cpp \
    src/uinputif.cpp \
    src/driverBase.cpp \
    src/tca8424driver.cpp \
    src/keymapping.cpp \
    src/eepromdriver.cpp \
    src/modifierhandler.cpp \
    src/uinputevpoll.cpp \
    ../dbus/src/daemonAdaptor.cpp \
    ../dbus/src/userdaemonInterface.cpp \
    ../dbus/src/settingsuiInterface.cpp

HEADERS += \
    src/toh.h \
    src/worker.h \
    src/tohkeyboard.h \
    src/uinputif.h \
    src/driverBase.h \
    src/tca8424driver.h \
    src/keymapping.h \
    src/defaultSettings.h \
    src/eepromdriver.h \
    src/modifierhandler.h \
    src/uinputevpoll.h \
    ../dbus/src/daemonAdaptor.h \
    ../dbus/src/userdaemonInterface.h \
    ../dbus/src/settingsuiInterface.h

OTHER_FILES += \
    config/layouts/$${TARGET}.conf \
    config/layouts/$${TARGET}.qml \
    config/icon-system-keyboard.png

