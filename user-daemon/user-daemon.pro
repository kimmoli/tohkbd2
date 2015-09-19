TARGET = harbour-tohkbd2-user

CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5 nemonotifications-qt5

QT += dbus gui-private

system(lupdate src qml -no-obsolete -ts $$PWD/i18n/engineering_en.ts)
system(lrelease -idbased $$PWD/i18n/*.ts)

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

translations.path = /usr/share/$${TARGET}/i18n
translations.files = i18n/*.qm

icons.path = /usr/share/$${TARGET}
icons.files = config/icon-system-keyboard.png \
              config/icon-lock-tohkbd2.png

notificationCategories.path = /usr/share/lipstick/notificationcategories
notificationCategories.files = config/x-harbour.tohkbd2.conf \
                               config/x-harbour.tohkbd2.screenshot.conf

INSTALLS += translations icons notificationCategories

message($${DEFINES})

SOURCES += \
    src/tohkbd2user.cpp \
    ../dbus/src/userdaemonAdaptor.cpp \
    src/userdaemon.cpp \
    src/viewhelper.cpp \
    src/applauncher.cpp \
    src/screenshot.cpp

HEADERS += \
    ../dbus/src/userdaemonAdaptor.h \
    src/userdaemon.h \
    src/viewhelper.h \
    src/applauncher.h \
    src/screenshot.h

RESOURCES += \
    config/keymaps.qrc

OTHER_FILES += \
    harbour-tohkbd2-user.desktop \
    config/icon-system-keyboard.png \
    config/icon-lock-tohkbd2.png \
    config/x-harbour.tohkbd2.conf \
    config/x-harbour.tohkbd2.screenshot.conf \
    i18n/*.ts \
    qml/taskswitcher.qml \
    config/layouts/*.tohkbdmap \
    config/x-harbour.tohkbd2.conf \
    config/x-harbour.tohkbd2.screenshot.conf

