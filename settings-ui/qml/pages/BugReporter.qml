/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    Rectangle
    {
        z:2
        anchors.fill: parent
        visible: bugReportPageOpen
        color: Qt.rgba(0, 0, 0, 0.5)

        BusyIndicator
        {
            id: bi
            size: BusyIndicatorSize.Large
            anchors.centerIn: parent
            running: bugReportPageOpen
        }
        Label
        {
            visible: bugReportPageOpen
            text: qsTr("Launching external application...")
            anchors.top: bi.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

    }

    SilicaFlickable
    {
        anchors.fill: parent
        z:1

        contentHeight: column.height

        Column
        {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader
            {
                title: qsTr("Report a bug")
            }
            Label
            {
                text: qsTr("Please first check TOHKBD FAQ page at https://together.jolla.com. Button below opens this link in browser.")
                color: Theme.primaryColor
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button
            {
                text: qsTr("Open FAQ")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked:
                {
                    bugReportPageOpen = true
                    Qt.openUrlExternally("https://together.jolla.com/question/96836/tohkbd-faq/")
                }
            }
            Label
            {
                text: qsTr("If there are no answer to your issue, please report it. Button below launches email client.")
                color: Theme.primaryColor
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button
            {
                text: qsTr("Report issue")
                anchors.horizontalCenter: column.horizontalCenter
                onClicked:
                {
                    bugReportPageOpen = true
                    Qt.openUrlExternally("mailto: toho@saunalahti.fi" +
                                         "?subject=Tohkbd2 bug report, ui " + settingsui.version + ", daemon " + daemonVersion + ", SFOS " + sailfishVersion +
                                         "&body=Write here what is wrong... (in Finnish or English)   ")
                }
            }


            Component.onCompleted: bugReportPageOpen = false

        }
    }

}

