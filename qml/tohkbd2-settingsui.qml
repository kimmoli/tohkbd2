/*
    tohkbd2-settingsui, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.tohkbd2.settingsui 1.0

ApplicationWindow
{
    id: apppage

    property string coverActionLeftIcon: "image://theme/icon-cover-pause"
    property string coverActionRightIcon: "image://theme/icon-cover-play"

    initialPage: Qt.resolvedUrl("pages/Tohkbd2Settings.qml")
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
            var i
            var tmp = settingsui.getApplications()
            for (i=0 ; i<tmp.length; i++)
            {
                applicationsModel.append({"name": tmp[i]["name"],
                                          "iconId": tmp[i]["iconId"],
                                          "filePath": tmp[i]["filePath"]})
            }

            tmp = settingsui.getCurrentShortcuts()
            for (i=0 ; i<tmp.length; i++)
            {
                shortcutsModel.append({"key": tmp[i]["key"],
                                      "name": tmp[i]["name"],
                                      "iconId": tmp[i]["iconId"],
                                      "filePath": tmp[i]["filePath"]})
            }

        }
    }

    ListModel
    {
        id: applicationsModel
    }

    ListModel
    {
        id: shortcutsModel
    }
}


