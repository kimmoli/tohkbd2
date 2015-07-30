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
        upDownItemCount: settingslist.count
        onKeyEnterPressed:
        {
            if (settingslist.get(upDownSelection).isEnabled)
            {
                pageStack.push(Qt.resolvedUrl(settingslist.get(upDownSelection).pageId))
            }
        }
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent

        PullDownMenu
        {
            MenuItem
            {
                //: Menu option and header for about page
                //% "About..."
                text: qsTrId("about")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),
                                          { "version": settingsui.version,
                                              "year": "2014-2015",
                                              //: Application name
                                              //% "TOHKBD Settings"
                                              "name": qsTrId("tohkbd2-settings-app-name"),
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
                //: Main page header
                //% "Settings"
                title: qsTrId("settings")
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
                    highlighted: down || kbdif.upDownSelection === index

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

                    onClicked:
                    {
                        kbdif.upDownSelection = index
                        pageStack.push(Qt.resolvedUrl(pageId))
                    }

                    onDownChanged: kbdif.upDownSelection = index
                }
            }

            Item
            {
                height: Theme.itemSizeSmall
                width: 1
            }

            ListItem
            {
                id: kr
                width: parent.width
                height: Theme.itemSizeSmall + kcm.height
                menu: kcm

                Image
                {
                    id: kimg
                    x: Theme.paddingLarge
                    source: kr.down ? "image://theme/icon-m-keyboard?" + Theme.highlightColor : "image://theme/icon-m-keyboard"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Label
                {

                    //: Prefix for showing current layout
                    //% "Keyboard layout: %1"
                    text: qsTrId("kbd-layout").arg(settings["physicalLayout"])
                    anchors.left: kimg.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.verticalCenter: parent.verticalCenter
                    color: kr.down ? Theme.highlightColor : Theme.primaryColor
                }
            }

            ContextMenu
            {
                id: kcm
                MenuItem
                {
                    //: Context menu entry for opening jolla-settings application
                    //% "Start Jolla Settings..."
                    text: qsTrId("start-jolla-settings")
                    onClicked: settingsui.startJollaSettings()
                }
                MenuItem
                {
                    //: Context menu entry for reloading keyboard mapping file for tohkbd
                    //% "Reload keyboard mapping"
                    text: qsTrId("force-reload")
                    onClicked: settingsui.forceKeymapReload()
                }
            }

            Label
            {
                //: Description text for sticky and locking modifier keys
                //% "To change keyboard layout, start Jolla Settings > System settings > Text input and change hardware keyboard active layout. Note that all layouts are not supported."
                text: qsTrId("layout-desc")
                x: Theme.paddingLarge
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                width: parent.width - 2*Theme.paddingLarge
            }
        }
    }

    ListModel
    {
        id: settingslist

        Component.onCompleted:
        {
            //: Main menu selection for shortcuts configurations
            //% "Shortcuts"
            settingslist.append({"labelId": qsTrId("shortcuts"),         "iconId":"image://theme/icon-m-shortcut",       "pageId":"Shortcuts.qml",       "isEnabled":(daemonVersion !== "N/A")})

            //: Main menu selection for general settings
            //% "General settings"
            settingslist.append({"labelId": qsTrId("general-settings"),  "iconId":"image://theme/icon-m-developer-mode", "pageId":"GeneralSettings.qml", "isEnabled":(daemonVersion !== "N/A")})

            //: Main menu selection for FAQ and reporting an issue, page header for bug reporter page. button text for triggering email app.
            //% "Report a bug"
            settingslist.append({"labelId": qsTrId("report-a-bug"),      "iconId":"image://theme/icon-m-crash-reporter", "pageId":"BugReporter.qml",     "isEnabled":true})
        }
    }

}


