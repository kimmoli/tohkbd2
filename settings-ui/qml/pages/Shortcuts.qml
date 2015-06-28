/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    SilicaFlickable
    {
        anchors.fill: parent
        VerticalScrollDecorator {}

        PullDownMenu
        {
            MenuItem
            {
                //: Pulldown menu optiion to reset default values
                //% "Reset all to defaults"
                text: qsTrId("reset-to-defaults")
                onClicked: settingsui.setShortcutsToDefault()
            }
        }

        contentHeight: column.height

        Column
        {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader
            {
                title: qsTrId("shortcuts")
            }

            Repeater
            {
                model: shortcutsModel
                ListItem
                {
                    id: shortcutItem

                    Image
                    {
                        id: keyFrame
                        property string sourceFile:"image://tohkbd2/icon-m-keyframe"
                        source: shortcutItem.highlighted ? sourceFile + "?" + Theme.highlightColor : sourceFile
                        anchors
                        {
                            left: parent.left
                            leftMargin: Theme.paddingMedium
                            verticalCenter: parent.verticalCenter
                        }

                        Label
                        {
                            id: keyName
                            anchors.centerIn: parent
                            font.pixelSize: Theme.fontSizeMedium
                            font.bold: true
                            text: key
                            color: shortcutItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                        }
                    }

                    Image
                    {
                        id: appIcon
                        source: iconId
                        y: Math.round((parent.height - height) / 2)
                        property real size: Theme.iconSizeLauncher

                        sourceSize.width: size
                        sourceSize.height: size
                        width: size
                        height: size

                        anchors
                        {
                            left: keyFrame.right
                            leftMargin: Theme.paddingMedium
                            verticalCenter: parent.verticalCenter
                        }
                    }
                    Image
                    {
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
                        anchors
                        {
                            left: appIcon.right
                            leftMargin: Theme.paddingLarge
                            verticalCenter: parent.verticalCenter
                        }
                        color: shortcutItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                        font.pixelSize: Theme.fontSizeMedium
                        truncationMode: TruncationMode.Fade
                        text: name
                    }

                    height: Theme.itemSizeMedium

                    onClicked: pageStack.push(appSelector, {"keyId": key})
                }

            }

        }
    }

    Component
    {
        id: appSelector
        ApplicationSelectionPage
        {
            onSelected:
            {
                console.log(keyId + " = " + filePath)
                settingsui.setShortcut(keyId, filePath)
            }
        }
    }


}


