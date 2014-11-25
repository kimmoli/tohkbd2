/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

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
                    fullimageview.width = 960
                    fullimageview.source = imagesource
                }

            }
        }
    }

    Image
    {
        id: fullimageview
        anchors.centerIn: parent
        width: 540
        height: 540
        fillMode: Image.PreserveAspectFit
        visible: width > 540
        Behavior on width { NumberAnimation {} }

        PinchArea
        {
            id: pinchArea
            property real minScale: 1.0
            property real maxScale: 2.5

            anchors.fill: parent
            enabled: zoomin
            pinch.target: fullimageview
            pinch.minimumScale: minScale * 0.5 // This is to create "bounce back effect"
            pinch.maximumScale: maxScale * 1.5 // when over zoomed


            onPinchFinished:
            {
                if (fullimageview.scale < pinchArea.minScale)
                {
                    bounceBackAnimation.to = pinchArea.minScale
                    bounceBackAnimation.start()
                }
                else if (fullimageview.scale > pinchArea.maxScale)
                {
                    bounceBackAnimation.to = pinchArea.maxScale
                    bounceBackAnimation.start()
                }
            }

            NumberAnimation
            {
                id: bounceBackAnimation
                target: fullimageview
                duration: 250
                property: "scale"
                from: fullimageview.scale
            }

            MouseArea
            {
                property real iX
                property real iY
                property real movementX : 0
                property real movementY : 0

                anchors.fill: parent
                enabled: zoomin

                onDoubleClicked:
                {
                    fullimageview.scale = 1
                    fullimageview.anchors.horizontalCenterOffset = 0
                    fullimageview.anchors.verticalCenterOffset = 0
                    fullimageview.width = 540
                    zoomin = false
                    pinchArea.scale = 1
                }

                onPressed:
                {
                    iX = mouseX
                    iY = mouseY
                }
                onPositionChanged:
                {
                    var dX = mouseX - iX
                    iX = mouseX
                    movementX += dX
                    var dY = mouseY - iY
                    iY = mouseY
                    movementY += dY

                    fullimageview.anchors.horizontalCenterOffset += movementX
                    fullimageview.anchors.verticalCenterOffset += movementY
                }
            }

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

            console.log("Theme.fontSizeHuge " + Theme.fontSizeHuge)
        }
    }
}


