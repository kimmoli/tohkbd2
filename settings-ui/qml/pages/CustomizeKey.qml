/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    property string label: "value"

    SilicaFlickable
    {
        anchors.fill: parent
        contentHeight: col.height
        Column
        {
            id: col
            width: parent.width
            spacing: Theme.paddingLarge

            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: label
            }

            Button
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "yepyep"
                onClicked: pageStack.pop()
            }

        }
    }
}

