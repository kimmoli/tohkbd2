/*
    Generic about page (C) 2014 Kimmo Lindholm
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.tohkbd2.ConsoleModel 1.0
import "../components"

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    property string name
    property string year
    property string version
    property string imagelocation

    onStatusChanged:
    {
        if (status === PageStatus.Inactive || status === PageStatus.Deactivating)
            aboutPageOpen = false
        else
            aboutPageOpen = true

        if (status === PageStatus.Activating)
            consoleModel.executeCommand("rpm", [ "-q", "--changelog", "harbour-ambience-tohkbd2" ])
    }

    BusyIndicator
    {
        id: busy
        size: BusyIndicatorSize.Large
        running: false
        visible: running
        anchors.centerIn: parent
        z: 100
    }

    KeyboardHandler
    {
        id: kbdif
        onKeyUpPressed: flick.flick(0, -1000)
        onKeyDownPressed: flick.flick(0, 1000)
        onKeyBackspacePressed: pageStack.pop()
    }

    ConsoleModel
    {
        id: consoleModel
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent

        contentHeight: column.height
        VerticalScrollDecorator { flickable: flick }

        Column
        {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader
            {
                title: qsTrId("about")
            }
            Label
            {
                x: Theme.paddingLarge
                text: name
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle
            {
                visible: imagelocation.length > 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: 120
                width: 120
                color: "transparent"

                Image
                {
                    visible: imagelocation.length > 0
                    source: imagelocation
                    anchors.centerIn: parent
                }
            }

            Label
            {
                text: "(C) " + year
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                //: header label for the team presentation
                //% "The TOHKBD2 Team"
                text: qsTrId("team-header")
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Column
            {
                spacing: Theme.paddingSmall
                width: parent.width

                Repeater
                {
                    model: authors
                    delegate: ListItem
                    {
                        id: author
                        width: parent.width
                        height: Theme.itemSizeMedium

                        onClicked:
                        {
                            busy.running = true
                            Qt.openUrlExternally(linkurl)
                        }

                        Rectangle
                        {
                            id: imgRect
                            color: "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.horizontalCenterOffset: -(540/2-Theme.paddingLarge-width/2)
                            height: parent.height
                            width: 110
                        }
                        Image
                        {
                            id: img
                            source: author.highlighted ? iconsource + "?" + Theme.highlightColor : iconsource
                            anchors.centerIn: imgRect
                        }
                        Label
                        {
                            id: lab
                            text: name
                            color: author.highlighted ? Theme.highlightColor : Theme.primaryColor
                            font.pixelSize: Theme.fontSizeMedium
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: img.right
                            anchors.leftMargin: Theme.paddingLarge
                        }
                    }
                }
            }

            Label
            {
                //: Version information field, for Settings UI application version
                //% "Settings UI version: "
                text: qsTrId("version-settings-ui") + version
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                //: Version information field, for daemon version (which runs as root)
                //% "Daemon version: "
                text: qsTrId("version-daemon") + daemonVersion
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                //: Version information field, for user daemon version (which runs as nemo)
                //% "User Daemon version: "
                text: qsTrId("version-user-daemon") + userDaemonVersion
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                //: Version information field, for Sailfish OS version
                //% "Sailfish version: "
                text: qsTrId("version-sailfish") + sailfishVersion
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            SectionHeader
            {
                //: section header for changelog
                //% "Changes"
                text: qsTrId("about-changes")
            }
            Column
            {
                width: parent.width - Theme.horizontalPageMargin
                x: Theme.horizontalPageMargin
                spacing: -Theme.fontSizeExtraSmall
                Repeater
                {
                    width: parent.width
                    model: consoleModel
                    delegate: Text
                    {
                        width: parent.width
                        text: modelData
                        textFormat: Text.PlainText
                        color: Theme.secondaryColor
                        wrapMode: Text.Wrap
                        font.pixelSize: Theme.fontSizeExtraSmall
                        font.bold: text.charAt(0) == "*"
                        visible: text.length > 1
                    }
                }
            }
        }
    }
    ListModel
    {
        id: authors
        ListElement
        {
            name: "Dirk \"dirkvl\" van Leersum"
            iconsource: "image://tohkbd2/dirkvl"
            linkurl: "http://www.funkyotherhalf.com"
        }
        ListElement
        {
            name: "Andrew \"wazd\" Zhilin"
            iconsource: "image://tohkbd2/wazd"
            linkurl: "https://www.behance.net/wazd"
        }
        ListElement
        {
            name: "Kimmo \"kimmoli\" Lindholm"
            iconsource: "image://tohkbd2/kimmoli"
            linkurl: "http://www.kimmoli.fi"
        }
    }
}
