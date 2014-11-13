TARGET = harbour-tohkbd2user

CONFIG += link_pkgconfig
PKGCONFIG += mlite5

QT += dbus
QT -= gui

# DBus
system(qdbusxml2cpp config/com.kimmoli.tohkbd2user.xml -i src/readerwriter.h -a src/adaptor)

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

systemd.path = /etc/systemd/user/
systemd.files = config/$${TARGET}.service

INSTALLS += target systemd

message($${DEFINES})

SOURCES += \
    src/tohkbd2user.cpp \
    src/readerwriter.cpp \
    src/adaptor.cpp

OTHER_FILES += \
    rpm/$${TARGET}.spec \
    config/$${TARGET}.service \
    config/com.kimmoli.tohkbd2user.xml

HEADERS += \
    src/readerwriter.h \
    src/adaptor.h
