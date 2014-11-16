#
# Project tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
#

TARGET = tohkbd2-settingsui

CONFIG += sailfishapp
CONFIG += link_pkgconfig
PKGCONFIG += mlite5 sailfishapp

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

SOURCES += src/tohkbd2-settingsui.cpp \
	src/settingsui.cpp
	
HEADERS += src/settingsui.h

OTHER_FILES += qml/tohkbd2-settingsui.qml \
    qml/cover/CoverPage.qml \
    qml/pages/Tohkbd2Settings.qml \
    qml/pages/AboutPage.qml \
    rpm/tohkbd2-settingsui.spec \
	tohkbd2-settingsui.png \
    tohkbd2-settingsui.desktop \
    qml/pages/ApplicationSelectionPage.qml

