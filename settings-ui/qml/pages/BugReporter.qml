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
            //: Text shown with busyindicator when starting external application
            //% "Launching external application..."
            text: qsTrId("launch-ext-app")
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
                title: qsTrId("report-a-bug")
            }
            Label
            {
                //: Description above open faq -button
                //% "Please first check TOHKBD FAQ page at https://together.jolla.com. Button below opens this link in browser."
                text: qsTrId("faq-text")
                color: Theme.primaryColor
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button
            {
                //: Button text, clicking this will open url showing FAQ
                //% "Open FAQ"
                text: qsTrId("faq-button")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked:
                {
                    bugReportPageOpen = true
                    Qt.openUrlExternally("https://together.jolla.com/question/96836/tohkbd-faq/")
                }
            }
            Label
            {
                //: Description above report issue -button
                //% "If there is no answer to your issue, please report it. Button below launches email client."
                text: qsTrId("report-text")
                color: Theme.primaryColor
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button
            {
                //: Button text, clicking this will launch email app
                //% "Report issue"
                text: qsTrId("report-button")
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

