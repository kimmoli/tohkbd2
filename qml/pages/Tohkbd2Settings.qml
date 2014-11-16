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
                                              "name": "The Otherhalf Keyboard 2 settings UI",
                                              "imagelocation": "/usr/share/icons/hicolor/86x86/apps/tohkbd2-settings-u.png"} )
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
                title: "Tohkbd2-settings-u"
            }
            Label
            {
                id: hoppa
                x: Theme.paddingLarge
                text: "Hello you"
                color: Theme.primaryColor
                // font.pixelSize: Theme.fontSizeExtraLarge
            }
            Button
            {
                text: "valitse"
                onClicked: pageStack.push(appSelector, {"filePaths": [], "currentIndex": -1})

            }

            Button
            {
                text: "starttaa"
                onClicked:
                {
                    settingsui.startApplication(appname)
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
                hoppa.text = filePath
                appname = filePath
                console.log(filePath)
            }
        }
    }


}


