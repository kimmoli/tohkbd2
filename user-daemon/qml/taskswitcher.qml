import QtQuick 2.1
import Sailfish.Silica 1.0

Item
{
    id: root

    width: Screen.width
    height: Screen.height


    function updateShortcutsModel()
    {
        var i
        var tmp = viewHelper.getCurrentShortcuts()

        shortcutsModel.clear()

        for (i=0 ; i<tmp.length; i++)
        {
            shortcutsModel.append({"iconId": tmp[i]["iconId"]})
        }
    }

    ListModel
    {
        id: shortcutsModel
    }

    Component.onCompleted: updateShortcutsModel()

    Rectangle
    {
        id: rect
        anchors.centerIn: root
        color: Theme.secondaryHighlightColor
        radius: 10
        width: 240
        height: 480

        Grid
        {
            anchors.centerIn: parent
            rotation: 90
            columns: 4
            rows: 2

            Repeater
            {
                model: shortcutsModel
                Rectangle
                {
                    color: viewHelper.currentApp === index ? Theme.highlightColor : "transparent"
                    radius: 10
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
                            onClicked: viewHelper.launchApplication(index)
                        }
                    }
                }
            }
        }
    }
}
