/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    backNavigation: false

    SilicaFlickable
    {
        width: 1600
        height: 500
        anchors.verticalCenter: parent.verticalCenter
        contentWidth: 1600
        contentHeight: 500


        GridView
        {
            id: sgv
            width: 1600
            height: 500
            model: 80

            anchors.verticalCenter: parent.verticalCenter

            cellHeight: 100
            cellWidth: 100

            delegate: Rectangle
            {
                width: 100
                height: 100
                border.width: 1
                border.color: "white"
                color: "transparent"

                Label
                {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    color: "blue"
                    text: index
                }
                Label
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    color: "white"
                    text: index
                }
            }
        }
    }
}

