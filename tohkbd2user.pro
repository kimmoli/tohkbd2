TARGET = harbour-tohkbd2user

CONFIG += link_pkgconfig
PKGCONFIG += mlite5

QT += dbus
QT -= gui

# DBus
system(qdbusxml2cpp config/com.kimmoli.tohkbd2user.xml -i src/readerwriter.h -a src/adaptor)

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

dbusService.files = config/com.kimmoli.tohkbd2user.service
dbusService.path = /usr/share/dbus-1/services/

dbusInterface.files = config/com.kimmoli.tohkbd2user.xml
dbusInterface.path = /usr/share/dbus-1/interfaces/

INSTALLS += target dbusService dbusInterface

message($${DEFINES})

SOURCES += \
    src/tohkbd2user.cpp \
    src/readerwriter.cpp \
    src/adaptor.cpp

OTHER_FILES += \
    rpm/$${TARGET}.spec \
    config/com.kimmoli.tohkbd2user.service \
    config/com.kimmoli.tohkbd2user.xml

HEADERS += \
    src/readerwriter.h \
    src/adaptor.h
