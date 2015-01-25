/*
    Generic about page (C) 2014 Kimmo Lindholm
*/

import QtQuick 2.0
import Sailfish.Silica 1.0


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

    SilicaFlickable
    {
        anchors.fill: parent

        contentHeight: column.height

        Column
        {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader
            {
                title: qsTr("About...")
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
                text: qsTr("The TOHKBD2 Team")
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
                text: qsTr("Settings UI version: ") + version
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                text: qsTr("Daemon version: ") + daemonVersion
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                text: qsTr("User daemon version: ") + userDaemonVersion
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
    ListModel
    {
        id: authors
        ListElement
        {
            name: "Dirk \"dirkvl\" VanLeersum"
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



