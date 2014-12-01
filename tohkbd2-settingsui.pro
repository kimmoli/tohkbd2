#
# Project tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
#

TARGET = harbour-tohkbd2-settingsui

QT += dbus

CONFIG += sailfishapp
CONFIG += link_pkgconfig
PKGCONFIG += mlite5 sailfishapp

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

SOURCES += src/tohkbd2-settingsui.cpp \
	src/settingsui.cpp
	
HEADERS += src/settingsui.h \
    src/IconProvider.h

OTHER_FILES += qml/tohkbd2-settingsui.qml \
    qml/cover/CoverPage.qml \
    qml/pages/Tohkbd2Settings.qml \
    qml/pages/AboutPage.qml \
    qml/pages/ApplicationSelectionPage.qml \
    qml/pages/Shortcuts.qml \
    qml/pages/KeyboardLayout.qml \
    qml/pages/CustomizeKeys.qml \
    qml/pages/BugReporter.qml \
    qml/pages/GeneralSettings.qml \
    qml/icons/icon-m-test.png \
    qml/images/layout_qwertz.png \
    qml/images/layout_qwertz.png \
    qml/images/layout_qwerty.png \
    qml/images/layout_nordic.png \
    qml/images/layout_cyrillic.png \
    qml/images/layout_azerty.png \
    qml/icons/icon-m-keyframe.png \
    harbour-tohkbd2-settingsui.desktop \
    harbour-tohkbd2-settingsui.png \
    rpm/harbour-tohkbd2-settingsui.spec

