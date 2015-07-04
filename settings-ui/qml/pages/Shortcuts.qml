/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page
{
    id: page

    KeyboardHandler
    {
        id: kbdif
        upDownItemCount: shortcutsModel.count
        onKeyUpPressed: flickRepeaterMover(flick, repeater)
        onKeyDownPressed: flickRepeaterMover(flick, repeater)
        onKeyEnterPressed:
        {
            pageStack.push(appSelector, {"keyId": shortcutsModel.get(upDownSelection).key})
        }

        onKeyBackspacePressed: pageStack.pop()

        Connections
        {
            target: page
            onOrientationTransitionRunningChanged: if (!orientationTransitionRunning) kbdif.flickRepeaterMover(flick, repeater)
        }
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent

        Behavior on contentY { NumberAnimation { duration: 200 } }

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
            spacing: Theme.paddingSmall
            PageHeader
            {
                id: pageheader
                title: qsTrId("shortcuts")
            }

            Repeater
            {
                id: repeater
                model: shortcutsModel

                ListItem
                {
                    id: shortcutItem
                    highlighted: down || kbdif.upDownSelection === index

                    height: Theme.itemSizeLarge

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

                    onClicked:
                    {
                        kbdif.upDownSelection = index
                        pageStack.push(appSelector, {"keyId": key})
                    }

                    onDownChanged: kbdif.upDownSelection = index
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


