/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted
    onOrientationChanged: if (zoomin) checkLimits()

    backNavigation: !zoomin

    property bool zoomin: false

    SlideshowView
    {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: (Theme.paddingLarge + 50)/2

        visible: !fullimageview.visible

        id: view
        width: 540
        height: 284
        itemWidth: 540

        model: layoutsmodel
        delegate: Item
        {
            width: view.itemWidth
            height: view.height + Theme.paddingLarge + 50
            Image
            {
                id: img
                source: imagesource
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
                text: layoutname
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeHuge
            }


            MouseArea
            {
                enabled: !zoomin && view.currentIndex == index
                anchors.fill: parent
                onClicked:
                {
                    zoomin = true
                    fullimageview.height = 540
                    fullimageview.anchors.horizontalCenterOffset = 0
                    fullimageview.source = imagesource
                }

            }
        }
    }

    Image
    {
        id: fullimageview
        anchors.centerIn: parent
        width: 960*(540/284)
        height: 284
        fillMode: Image.PreserveAspectFit
        visible: height > 284
        Behavior on height { NumberAnimation {} }

        MouseArea
        {
            property real iX
            property real movementX : 0

            anchors.fill: parent
            enabled: zoomin

            onDoubleClicked:
            {
                fullimageview.anchors.horizontalCenterOffset = 0
                fullimageview.height = 284
                zoomin = false
            }

            onPressed:
            {
                iX = mouseX
            }
            onPositionChanged:
            {
                var dX = mouseX - iX
                iX = mouseX
                movementX += dX

                fullimageview.anchors.horizontalCenterOffset += movementX
            }
            onReleased:
            {
                checkLimits()
            }
        }
    }

    NumberAnimation
    {
        id: bounceBackAnimation

        target: fullimageview
        duration: 250
        property: "anchors.horizontalCenterOffset"
        from: fullimageview.anchors.horizontalCenterOffset
    }

    function checkLimits()
    {
        var limits = fullimageview.width/2-page.width/2
        if (fullimageview.anchors.horizontalCenterOffset < -limits)
        {
            bounceBackAnimation.to = -limits
            bounceBackAnimation.start()
        }
        if (fullimageview.anchors.horizontalCenterOffset > limits)
        {
            bounceBackAnimation.to = limits
            bounceBackAnimation.start()
        }
    }


    ListModel
    {
        id: layoutsmodel

        Component.onCompleted:
        {
            layoutsmodel.append({"imagesource":"../images/layout_qwerty.png", "layoutname":"QWERTY"})
            layoutsmodel.append({"imagesource":"../images/layout_qwertz.png", "layoutname":"QWERTZ"})
            layoutsmodel.append({"imagesource":"../images/layout_azerty.png", "layoutname":"AZERTY"})
            layoutsmodel.append({"imagesource":"../images/layout_nordic.png", "layoutname":"Nordic"})
            layoutsmodel.append({"imagesource":"../images/layout_cyrillic.png", "layoutname":"Cyrillic"})
        }
    }
}


