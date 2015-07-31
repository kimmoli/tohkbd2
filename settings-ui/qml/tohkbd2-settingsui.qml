/*
    tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.tohkbd2.settingsui 1.0

ApplicationWindow
{
    id: apppage

    _defaultPageOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    onApplicationActiveChanged:
    {
        if (!applicationActive && viewMode === "--showhelp")
            Qt.quit()

        if (!applicationActive && bugReportPageOpen)
        {
            bugReportPageOpen = false
            pageStack.pop()
        }
        if (!applicationActive && aboutPageOpen)
        {
            aboutPageOpen = false
            pageStack.pop()
        }
    }

    property string coverActionLeftIcon: "image://theme/icon-cover-pause"
    property string coverActionRightIcon: "image://theme/icon-cover-play"
    property string daemonVersion : "---"
    property string userDaemonVersion : "---"
    property string sailfishVersion : "---"

    property bool bugReportPageOpen: false
    property bool aboutPageOpen: false
    property bool helpPageOpen: false

    property var settings

    initialPage:
    {
        if (viewMode === "--showhelp")
            return Qt.resolvedUrl("pages/Help.qml")
        else
            return Qt.resolvedUrl("pages/Tohkbd2Settings.qml")
    }

    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    function coverActionLeft()
    {
        console.log("Left cover action")
    }

    function coverActionRight()
    {
        console.log("Right cover action")
    }

    SettingsUi
    {
        id: settingsui

        Component.onCompleted:
        {
            daemonVersion = readDaemonVersion()
            userDaemonVersion = readUserDaemonVersion()
            sailfishVersion = readSailfishVersion()
            updateApplicationsModel()
            updateShortcutsModel()
            settings = settingsui.getCurrentSettings()
            updateKeycomboModel()
        }

        onShortcutsChanged:
        {
            updateShortcutsModel()
        }

        onSettingsChanged:
        {
            settings = settingsui.getCurrentSettings()
        }

        onShowHelpPage: if (viewMode !== "--showhelp" && !helpPageOpen)
                            pageStack.push(Qt.resolvedUrl("pages/Help.qml"))
    }

    ListModel
    {
        id: applicationsModel
    }

    ListModel
    {
        id: shortcutsModel
    }

    function updateApplicationsModel()
    {
        var i
        var tmp = settingsui.getApplications()

        applicationsModel.clear()

        for (i=0 ; i<tmp.length; i++)
        {
            applicationsModel.append({"name": tmp[i]["name"],
                                      "iconId": tmp[i]["iconId"],
                                      "filePath": tmp[i]["filePath"],
                                      "isAndroid": tmp[i]["isAndroid"]})
        }
    }

    function updateShortcutsModel()
    {
        var i
        var tmp = settingsui.getCurrentShortcuts()

        shortcutsModel.clear()

        for (i=0 ; i<tmp.length; i++)
        {
            shortcutsModel.append({"key": tmp[i]["key"],
                                  "name": tmp[i]["name"],
                                  "iconId": tmp[i]["iconId"],
                                  "filePath": tmp[i]["filePath"],
                                  "isAndroid": tmp[i]["isAndroid"]})
        }
    }

    ListModel
    {
        id: keycomboModel
    }

    function updateKeycomboModel()
    {
        //: Help page text (keep short)
        //% "Switch apps"
        keycomboModel.append({"key1": "Alt",   "key2": "Tab",        "key3": "",  "name": qsTrId("help-switch-app")})
        //: Help page text (keep short)
        //% "Screenshot"
        keycomboModel.append({"key1": "Sym",   "key2": "Ins",        "key3": "",  "name": qsTrId("help-screeshot")})
        //: Help page text (keep short)
        //% "Selfie LED"
        keycomboModel.append({"key1": "Sym",   "key2": "Del",        "key3": "",  "name": qsTrId("help-selfie-led")})
        //: Help page text (keep short)
        //% "Toggle backlight"
        keycomboModel.append({"key1": "Sym",   "key2": "Home",       "key3": "",  "name": qsTrId("help-toggle-backlight")})
        //: Help page text (keep short)
        //% "Decrease volume"
        keycomboModel.append({"key1": "Sym",   "key2": "Left",       "key3": "",  "name": qsTrId("help-dec-volume")})
        //: Help page text (keep short)
        //% "Increase volume"
        keycomboModel.append({"key1": "Sym",   "key2": "Right",      "key3": "",  "name": qsTrId("help-inc-volume")})
        //: Help page text (keep short)
        //% "New Email"
        keycomboModel.append({"key1": "Sym",   "key2": "@",          "key3": "",  "name": qsTrId("help-new-email")})
        //: Help page text (keep short)
        //% "Toggle CapsLock"
        keycomboModel.append({"key1": "Sym",   "key2": "Left-Shift", "key3": "",  "name": qsTrId("help-capslock")})

        //: Help page text (keep short)
        //% "Select text"
        keycomboModel.append({"key1": "Shift", "key2": "Arrows",     "key3": "",  "name": qsTrId("help-select")})
        //: Help page text (keep short)
        //% "Select all"
        keycomboModel.append({"key1": "Ctrl",  "key2": "A",          "key3": "",  "name": qsTrId("help-select-all")})
        //: Help page text (keep short)
        //% "Copy"
        keycomboModel.append({"key1": "Ctrl",  "key2": "C",          "key3": "",  "name": qsTrId("help-copy")})
        //: Help page text (keep short)
        //% "Cut"
        keycomboModel.append({"key1": "Ctrl",  "key2": "X",          "key3": "",  "name": qsTrId("help-cut")})
        //: Help page text (keep short)
        //% "Paste"
        keycomboModel.append({"key1": "Ctrl",  "key2": "V",          "key3": "",  "name": qsTrId("help-paste")})
        //: Help page text (keep short)
        //% "Undo"
        keycomboModel.append({"key1": "Ctrl",  "key2": "Z",          "key3": "",  "name": qsTrId("help-undo")})
        //: Help page text (keep short)
        //% "Redo"
        keycomboModel.append({"key1": "Ctrl",  "key2": "Shift",      "key3": "Z", "name": qsTrId("help-redo")})
        //: Help page text (keep short)
        //% "Delete word"
        keycomboModel.append({"key1": "Ctrl",  "key2": "Backspace",  "key3": "",  "name": qsTrId("help-delete-word")})

        //: Help page text (keep short)
        //% "Reboot phone"
        keycomboModel.append({"key1": "Ctrl",  "key2": "Alt",        "key3": "Del",       "name": qsTrId("help-reboot-phone")})
        //: Help page text (keep short)
        //% "Restart lipstick"
        keycomboModel.append({"key1": "Ctrl",  "key2": "Alt",        "key3": "Backspace", "name": qsTrId("help-restart-lipstick")})
    }
}


