TARGET = harbour-tohkbd2

QT += dbus
QT -= gui

# D-bus adaptor and interface
system(qdbusxml2cpp config/com.kimmoli.tohkbd2.xml -i src/tohkeyboard.h -a src/daemonAdaptor)
system(qdbusxml2cpp config/com.kimmoli.tohkbd2.xml -p ../daemon/src/daemonInterface)

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

systemd.path = /etc/systemd/system/
systemd.files = config/$${TARGET}.service

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/$${TARGET}.conf

vkblayout.path = /usr/share/maliit/plugins/com/jolla/layouts/
vkblayout.files = config/layouts/$${TARGET}.conf config/layouts/$${TARGET}.qml

INSTALLS += target systemd udevrule dbusconf vkblayout

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
    src/daemonAdaptor.cpp \
    src/eepromdriver.cpp \
    src/modifierhandler.cpp \
    ../user-daemon/src/userInterface.cpp \
    ../settings-ui/src/settingsuiInterface.cpp \
    src/uinputevpoll.cpp

HEADERS += \
    src/toh.h \
    src/worker.h \
    src/tohkeyboard.h \
    src/uinputif.h \
    src/driverBase.h \
    src/tca8424driver.h \
    src/keymapping.h \
    src/daemonAdaptor.h \
    src/defaultSettings.h \
    src/eepromdriver.h \
    src/modifierhandler.h \
    ../user-daemon/src/userInterface.h \
    ../settings-ui/src/settingsuiInterface.h \
    src/uinputevpoll.h

OTHER_FILES += \
    config/$${TARGET}.service \
    config/$${TARGET}.conf \
    config/layouts/$${TARGET}.conf \
    config/layouts/$${TARGET}.qml \
    config/com.kimmoli.tohkbd2.xml \
    config/icon-system-keyboard.png

