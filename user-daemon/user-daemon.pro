TARGET = harbour-tohkbd2-user

CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5 nemonotifications-qt5

QT += dbus gui-private

system(qdbusxml2cpp config/com.kimmoli.tohkbd2user.xml -i src/userdaemon.h -a src/userAdaptor)
system(qdbusxml2cpp config/com.kimmoli.tohkbd2user.xml -p ../user-daemon/src/userInterface)

#system(lupdate src qml -no-obsolete -ts $$PWD/i18n/engineering_en.ts)
system(lrelease -idbased $$PWD/i18n/*.ts)

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

dbusService.files = config/com.kimmoli.tohkbd2user.service
dbusService.path = /usr/share/dbus-1/services/

dbusInterface.files = config/com.kimmoli.tohkbd2user.xml
dbusInterface.path = /usr/share/dbus-1/interfaces/

translations.path = /usr/share/$${TARGET}/i18n
translations.files = i18n/*.qm

icons.path = /usr/share/$${TARGET}
icons.files = config/icon-system-keyboard.png

INSTALLS +=  dbusService dbusInterface translations icons

message($${DEFINES})

SOURCES += \
    src/tohkbd2user.cpp \
    src/userAdaptor.cpp \
    src/userdaemon.cpp \
    src/viewhelper.cpp \
    src/applauncher.cpp \
    src/screenshot.cpp

OTHER_FILES += \
    config/com.kimmoli.tohkbd2user.service \
    config/com.kimmoli.tohkbd2user.xml \
    config/icon-system-keyboard.png \
    i18n/*.ts \
    qml/taskswitcher.qml \
    config/layouts/*.tohkbdmap

HEADERS += \
    src/userAdaptor.h \
    src/userdaemon.h \
    src/viewhelper.h \
    src/applauncher.h \
    src/screenshot.h

RESOURCES += \
    config/keymaps.qrc
