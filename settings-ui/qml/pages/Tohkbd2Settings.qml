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
        upDownItemCount: kr.menuOpen ? kr.menu.children.length : settingslist.count + 1

        onUpDownSelectionChanged:
        {
            if (kr.menuOpen)
            {
                if (kr.menu.children[upDownSelection])
                    kr.menu._setHighlightedItem(kr.menu.children[upDownSelection])
            }
            else
            {
                if ((upDownSelection === (upDownItemCount - 1)) && (flick.contentHeight > flick.height))
                    flick.scrollToBottom()
            }
        }

        onKeyEnterPressed:
        {
            if (kr.menuOpen)
            {
                if (kr.menu.children[upDownSelection])
                    kr.menu._activatedMenuItem(kr.menu.children[upDownSelection])
                upDownSelection = -1
            }
            else if (upDownSelection === (upDownItemCount - 1))
            {
                if (!kr.menuOpen)
                {
                    updateLayouts()
                    kr.showMenu()
                    upDownSelection = 0
                }
            }
            else
            {
                if (settingslist.get(upDownSelection).isEnabled && !kr.menuOpen)
                {
                    pageStack.push(Qt.resolvedUrl(settingslist.get(upDownSelection).pageId))
                }
            }
        }

        onKeyBackspacePressed:
        {
            if (kr.menuOpen) kr.hideMenu()
            upDownSelection = -1
        }
    }

    RemorsePopup
    {
        id: remorse
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent

        PullDownMenu
        {
            MenuItem
            {
                //: Menu option to restart daemon
                //% "Restart daemon"
                text: qsTrId("restart-daemon")
                onClicked: settingsui.quitDaemon()
            }
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
                id: repeater
                model: settingslist

                ListItem
                {
                    id: listItem
                    height: Theme.itemSizeSmall
                    enabled: isEnabled
                    opacity: enabled ? 1.0 : 0.4
                    highlighted: (down || kbdif.upDownSelection === index) && !kr.menuOpen

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

            ListItem
            {
                id: kr
                width: parent.width
                height: Theme.itemSizeSmall + kcm.height
                highlighted: down || menuOpen || kbdif.upDownSelection === (kbdif.upDownItemCount - 1)
                menu: kcm
                showMenuOnPressAndHold: false
                onClicked:
                {
                    kbdif.upDownSelection = (kbdif.upDownItemCount - 1)
                    updateLayouts()
                    showMenu()
                }

                Image
                {
                    id: kimg
                    x: Theme.paddingLarge
                    source: kr.highlighted ? "image://theme/icon-m-keyboard?" + Theme.highlightColor : "image://theme/icon-m-keyboard"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Label
                {
                    id: kprfx
                    //: Prefix for showing current layout
                    //% "Keyboard layout"
                    text: qsTrId("kbd-layout")
                    anchors.left: kimg.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.verticalCenter: parent.verticalCenter
                    color: kr.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
                Label
                {
                    text: settings["physicalLayout"]
                    anchors.left: kprfx.right
                    anchors.leftMargin: Theme.paddingMedium
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.highlightColor
                }
            }

            ContextMenu
            {
                id: kcm
                MenuItem
                {
                    //: Context menu entry for changing the layout
                    //% "Change layout..."
                    text: qsTrId("change-layout")
                    onClicked: pageStack.push(Qt.resolvedUrl("LayoutSwitcher.qml"))
                }
                MenuItem
                {
                    //: Context menu entry for reloading keyboard mapping file for tohkbd
                    //% "Reload keyboard mapping"
                    text: qsTrId("force-reload")
                    onClicked: settingsui.forceKeymapReload()
                }
                MenuItem
                {
                    //: Context menu entry for overwrite keyboard mapping files with original ones
                    //% "Restore original keymap files"
                    text: qsTrId("reset-keymaps")
                    onClicked: remorse.execute(qsTrId("reset-keymaps"),
                                               function() { settingsui.restoreOriginalKeymaps() } )
                }
            }

            Label
            {
                //: Description text for sticky and locking modifier keys
                //% "To change keyboard layout, click above and select 'Change layout'. Unsupported layouts are dimmed."
                text: qsTrId("layout-desc")
                x: Theme.paddingLarge
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
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

            //: Main menu selection for Help page with keyboard shortcuts etc.
            //% "Help"
            settingslist.append({"labelId": qsTrId("help"),              "iconId":"image://theme/icon-m-question",       "pageId":"Help.qml",            "isEnabled":true})
        }
    }

}


