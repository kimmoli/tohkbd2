import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    property string keyId: ""

    signal selected(string keyId, string filePath)

    SilicaGridView
    {
        id: gridView
        VerticalScrollDecorator {}

        anchors.fill: parent
        cellWidth: page.width / ((orientation === Orientation.Portrait) ? 4 : 7)
        cellHeight: Screen.height / 6
        header: Item
        {
            height: Screen.height / 6
            width: page.width
            PageHeader
            {
                //: Application shortcut selector page header
                //% "Select application"
                title: qsTrId("select-application")
            }
        }

        model: applicationsModel

        delegate: Rectangle
        {
            id: appItem
            color: "Transparent"

            width: gridView.cellWidth
            height: gridView.cellHeight
            Image
            {
                id: appIcon
                source: iconId
                y: Math.round((parent.height - (height + appTitle.height)) / 2)
                anchors.horizontalCenter: parent.horizontalCenter
                property real size: Theme.iconSizeLauncher

                sourceSize.width: size
                sourceSize.height: size
                width: size
                height: size

            }
            Image
            {
                id: androidIcon
                anchors
                {
                    top: appIcon.top
                    right: appIcon.right
                    topMargin: -16
                    rightMargin: -16
                }
                visible: isAndroid
                source: "image://theme/icon-s-android"
            }
            Label
            {
                id: appTitle
                text: name
                anchors
                {
                    top: appIcon.bottom
                    topMargin: Theme.paddingSmall
                    left: parent.left
                    right: parent.right
                }
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeTiny
                elide: Text.ElideRight
                color: Theme.primaryColor
            }
            Label
            {
                id: isShortcut
                text: ""
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                anchors.centerIn: appIcon
                Component.onCompleted:
                {
                    var i
                    for (i = 0 ; i< shortcutsModel.count ; i++)
                        if (shortcutsModel.get(i).filePath === filePath)
                        {
                            isShortcut.text = shortcutsModel.get(i).key
                            appIcon.opacity = 0.4
                            androidIcon.opacity = 0.4
                            bgItem.enabled = false
                        }
                }
            }

            BackgroundItem
            {
                id: bgItem
                anchors.fill: parent
                onClicked:
                {
                    selected(keyId, filePath)
                    pageStack.pop()
                }

            }

        }
    }
}

