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

        PullDownMenu
        {
            MenuItem
            {
                text: "About..."
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),
                                          { "version": settingsui.version,
                                              "year": "2014",
                                              "name": "TOHKBD2 Settings",
                                              "imagelocation": "/usr/share/icons/hicolor/86x86/apps/harbour-tohkbd2-settingsui.png"} )
            }
        }

        contentHeight: column.height

        Column
        {
            id: column
            width: page.width

            PageHeader
            {
                title: "Settings"
            }

            Label
            {
                x: Theme.paddingLarge
                width: parent.width - 2*Theme.paddingLarge
                text: "Daemon version: " + daemonVersion
                truncationMode: TruncationMode.Fade
            }

            Item
            {
                height: Theme.itemSizeSmall
                width: 1
            }


            Repeater
            {
                model: settingslist

                ListItem
                {
                    id: listItem
                    height: Theme.itemSizeSmall
                    enabled: isEnabled
                    opacity: enabled ? 1.0 : 0.4

                    Image
                    {
                        id: name
                        x: Theme.paddingLarge
                        source: listItem.highlighted ? iconId + "?" + Theme.highlightColor : iconId
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Label
                    {
                        text: labelId
                        anchors.left: name.right
                        anchors.leftMargin: Theme.paddingLarge
                        anchors.verticalCenter: parent.verticalCenter
                        color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }

                    onClicked: pageStack.push(Qt.resolvedUrl(pageId))
                }
            }
        }
    }

    ListModel
    {
        id: settingslist

        Component.onCompleted:
        {
            settingslist.append({"labelId":"Shortcuts",         "iconId":"image://theme/icon-m-shortcut",       "pageId":"Shortcuts.qml",       "isEnabled":(daemonVersion !== "N/A")})
            settingslist.append({"labelId":"Layout",            "iconId":"image://theme/icon-m-keyboard",       "pageId":"KeyboardLayout.qml",  "isEnabled":(daemonVersion !== "N/A")})
            settingslist.append({"labelId":"Customize mapping", "iconId":"image://tohkbd2/icon-m-test",         "pageId":"CustomizeKeys.qml",   "isEnabled":(daemonVersion !== "N/A")})
            settingslist.append({"labelId":"General settings",  "iconId":"image://tohkbd2/icon-m-test",         "pageId":"GeneralSettings.qml", "isEnabled":(daemonVersion !== "N/A")})
            settingslist.append({"labelId":"Report a bug",      "iconId":"image://theme/icon-m-crash-reporter", "pageId":"BugReporter.qml",     "isEnabled":true})
        }
    }

}


