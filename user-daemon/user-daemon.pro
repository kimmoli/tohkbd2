TARGET = harbour-tohkbd2-user

CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5

QT += dbus gui-private

# DBus
system(qdbusxml2cpp config/com.kimmoli.tohkbd2user.xml -i src/userdaemon.h -a src/adaptor)

#system(lupdate src -ts $$PWD/i18n/*.ts)
system(lrelease $$PWD/i18n/*.ts)


DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

#target.path = /usr/bin/

dbusService.files = config/com.kimmoli.tohkbd2user.service
dbusService.path = /usr/share/dbus-1/services/

dbusInterface.files = config/com.kimmoli.tohkbd2user.xml
dbusInterface.path = /usr/share/dbus-1/interfaces/

translations.path = /usr/share/$${TARGET}/i18n
translations.files = i18n/translations_*.qm

icons.path = /usr/share/$${TARGET}
icons.files = config/icon-system-keyboard.png

INSTALLS +=  dbusService dbusInterface translations icons
#target

message($${DEFINES})

SOURCES += \
    src/tohkbd2user.cpp \
    src/adaptor.cpp \
    src/userdaemon.cpp \
    src/viewhelper.cpp

OTHER_FILES += \
    config/com.kimmoli.tohkbd2user.service \
    config/com.kimmoli.tohkbd2user.xml \
    config/icon-system-keyboard.png \
    i18n/translations_en.ts \
    i18n/translations_fi.ts \
    i18n/translations_nl.ts \
    i18n/translations_de.ts \
    i18n/translations_da.ts \
    i18n/translations_fr.ts \
    i18n/translations_it.ts \
    i18n/translations_no.ts \
    i18n/translations_sv.ts \
    qml/taskswitcher.qml

HEADERS += \
    src/adaptor.h \
    src/userdaemon.h \
    src/viewhelper.h
