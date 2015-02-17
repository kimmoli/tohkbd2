import QtQuick 2.1
import Sailfish.Silica 1.0
import harbour.tohkbd2.taskswitcher 1.0

Item
{
    id: root

    width: Screen.width
    height: Screen.height


    function updateShortcutsModel()
    {
        var i
        var tmp = taskswitcher.getCurrentShortcuts()

        shortcutsModel.clear()

        for (i=0 ; i<tmp.length; i++)
        {
            shortcutsModel.append({"key": tmp[i]["key"],
                                  "name": tmp[i]["name"],
                                  "iconId": tmp[i]["iconId"], /* this */
                                  "filePath": tmp[i]["filePath"], /* and this we need... */
                                  "isAndroid": tmp[i]["isAndroid"]})
        }
    }

    ListModel
    {
        id: shortcutsModel
    }

    Taskswitcher
    {
        id: taskswitcher

        Component.onCompleted: updateShortcutsModel()
    }

    Rectangle
    {
        id: rect
        anchors.centerIn: root
        color: "green"
        radius: 10
        width: 480
        height: 240
        opacity: 0.8

        Grid
        {
            anchors.centerIn: parent
            columns: 4
            rows: 2

            Repeater
            {
                model: shortcutsModel
                Rectangle
                {
                    color: "transparent"
                    width: 120
                    height: 120

                    Image
                    {
                        anchors.centerIn: parent
                        id: appIcon
                        source: iconId
                        y: Math.round((parent.height - height) / 2)
                        property real size: Theme.iconSizeLauncher

                        sourceSize.width: size
                        sourceSize.height: size
                        width: size
                        height: size

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                taskswitcher.launchApplication(filePath)
                                viewHelper.hideWindow()
                            }
                        }
                    }
                }
            }
        }
    }
}
