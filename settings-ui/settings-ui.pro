#
# Project tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
#

TARGET = harbour-tohkbd2-settingsui

QT += dbus

CONFIG += sailfishapp
CONFIG += link_pkgconfig
PKGCONFIG += mlite5 Qt5SystemInfo sailfishapp

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

system(lupdate src qml -no-obsolete -ts $$PWD/i18n/engineering_en.ts)
system(lrelease -idbased $$PWD/i18n/*.ts)

translations.path = /usr/share/$${TARGET}/i18n
translations.files = i18n/*.qm

INSTALLS += translations

message($${DEFINES})

SOURCES += src/tohkbd2-settingsui.cpp \
        src/settingsui.cpp \
        src/consolemodel.cpp \
        ../dbus/src/settingsuiAdaptor.cpp \
        ../dbus/src/daemonInterface.cpp \
        ../dbus/src/userdaemonInterface.cpp
	
HEADERS += src/settingsui.h \
        src/IconProvider.h \
        src/consolemodel.h \
        ../dbus/src/settingsuiAdaptor.h \
        ../dbus/src/daemonInterface.h \
        ../dbus/src/userdaemonInterface.h

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
    qml/components/KeyboardHandler.qml \
    qml/pages/Help.qml \
    qml/cover/HelpCoverPage.qml \
    qml/pages/LayoutSwitcher.qml
