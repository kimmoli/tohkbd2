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

        contentHeight: column.height

        Column
        {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader
            {
                title: "Report a bug"
            }
            Label
            {
                text: "Launching email application..."
                anchors.horizontalCenter: column.horizontalCenter
            }

            ProgressBar
            {
                width: page.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: column.horizontalCenter
                indeterminate: true
            }

        }
    }

    Component.onCompleted:
    {
        bugReportPageOpen = true
        Qt.openUrlExternally("mailto: toho@saunalahti.fi" +
                                     "?subject=Tohkbd2 bug report" +
                                     "&body=Write here what is wrong...")
    }


}

