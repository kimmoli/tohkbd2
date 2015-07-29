#
# Project tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
#

TARGET = harbour-tohkbd2-settingsui

QT += dbus

CONFIG += sailfishapp
CONFIG += link_pkgconfig
PKGCONFIG += mlite5 sailfishapp

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

system(lupdate src qml -no-obsolete -ts $$PWD/i18n/engineering_en.ts)
system(lrelease -idbased $$PWD/i18n/*.ts)

translations.path = /usr/share/$${TARGET}/i18n
translations.files = i18n/*.qm

INSTALLS += translations

message($${DEFINES})

SOURCES += src/tohkbd2-settingsui.cpp \
        src/settingsui.cpp \
        ../daemon/src/daemonInterface.cpp \
        ../user-daemon/src/userInterface.cpp
	
HEADERS += src/settingsui.h \
        src/IconProvider.h \
        ../daemon/src/daemonInterface.h \
        ../user-daemon/src/userInterface.h

OTHER_FILES += qml/tohkbd2-settingsui.qml \
    qml/cover/CoverPage.qml \
    qml/pages/Tohkbd2Settings.qml \
    qml/pages/AboutPage.qml \
    qml/pages/ApplicationSelectionPage.qml \
    qml/pages/Shortcuts.qml \
    qml/pages/BugReporter.qml \
    qml/pages/GeneralSettings.qml \
    qml/icons/icon-m-test.png \
    qml/icons/icon-m-keyframe.png \
    harbour-tohkbd2-settingsui.desktop \
    harbour-tohkbd2-settingsui.png \
    i18n/*.ts \
    qml/icons/kimmoli.png \
    qml/icons/wazd.png \
    qml/icons/dirkvl.png \
    qml/components/KeyboardHandler.qml

