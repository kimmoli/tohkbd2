/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    property string appname: ""

    SilicaFlickable
    {
        anchors.fill: parent

        PullDownMenu
        {
            MenuItem
            {
                text: "About..."
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),
                                          { "version": settingsui.version,
                                              "year": "2014",
                                              "name": "TOHKBD2 Settings",
                                              "imagelocation": "/usr/share/icons/hicolor/86x86/apps/tohkbd2-settingsui.png"} )
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
                title: "Shortcuts"
            }

            Repeater
            {
                model: shortcutsModel
                ListItem
                {
                    id: shortcutItem

                    Label
                    {
                        id: keyName
                        anchors
                        {
                            left: parent.left
                            leftMargin: Theme.paddingMedium
                            verticalCenter: parent.verticalCenter
                        }
                        width: Theme.itemSizeExtraSmall
                        font.pixelSize: Theme.fontSizeMedium
                        font.bold: true
                        text: key
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
                            left: keyName.right
                            leftMargin: Theme.paddingMedium
                            verticalCenter: parent.verticalCenter
                        }

                    }
                    Label
                    {
                        anchors
                        {
                            left: appIcon.right
                            leftMargin: Theme.paddingLarge
//                            right: column.right
//                            rightMargin: Theme.paddingMedium
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


