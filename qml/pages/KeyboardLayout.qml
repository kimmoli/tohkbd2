/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    property bool zoomin: false

    SlideshowView
    {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: (Theme.paddingLarge + 50)/2

        visible: !fullimage.visible

        id: view
        width: 540
        height: 284
        itemWidth: 540

        model: 5
        delegate: Item
        {
            width: view.itemWidth
            height: view.height + Theme.paddingLarge + 50
            Image
            {
                id: img
                source: "../images/layout_qwertz.png"
                width: parent.width - Theme.paddingLarge
                height: view.height
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
            }
            Text
            {
                anchors.top: img.bottom
                anchors.topMargin: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                text: index == 0 ? "QWERTZ" : "item " + index
                color: "white"
                font.pixelSize: 50
            }


            MouseArea
            {
                enabled: !zoomin
                anchors.fill: parent
                onClicked:
                {
                    zoomin = true
                    fullimage.width = 960
                }

            }
        }
    }

    Image
    {
        id: fullimage
        anchors.centerIn: parent
        width: 540
        height: 540
        fillMode: Image.PreserveAspectFit
        source: "../images/layout_qwertz.png"
        visible: width>540

        Behavior on width { NumberAnimation {} }

        MouseArea
        {
            enabled: zoomin
            anchors.fill: parent
            onClicked:
            {
                zoomin = false
                fullimage.width = 540
            }
        }
    }
}

