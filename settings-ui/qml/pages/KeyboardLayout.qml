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

        visible: !fullimageview.visible

        id: view
        width: page.width
        height: 284
        itemWidth: 540
        clip: true

        model: layoutsmodel

        delegate: Item
        {
            width: view.itemWidth
            height: view.height
            Image
            {
                id: img
                source: imagesource
                width: 540 - Theme.paddingLarge
                height: view.height
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
            }

            MouseArea
            {
                enabled: !zoomin && view.currentIndex == index
                anchors.fill: parent
                onClicked:
                {
                    zoomin = true
                    fullimageview.height = 540
                    //fullimageview.anchors.horizontalCenterOffset = 0
                    fullimageview.source = imagesource
                    fullimageviewFlickable.contentX = (fullimageview.width - page.width)/2
                }
            }
        }
    }
    Text
    {
        id: layoutlabel
        anchors.top: view.bottom
        anchors.topMargin: Theme.paddingLarge
        anchors.horizontalCenter: page.horizontalCenter
        text: layoutsmodel.count < 5 ? "" : layoutsmodel.get(view.currentIndex).layoutname
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeLarge
        opacity: fullimageview.visible ? 0 : 1

        Behavior on opacity { NumberAnimation {} }
    }

    SilicaFlickable
    {
        id: fullimageviewFlickable
        visible: fullimageview.height > 284
        height: fullimageview.height
        width: parent.width

        contentHeight: fullimageview.height
        contentWidth: fullimageview.width
        anchors.verticalCenter: parent.verticalCenter

        Image
        {
            id: fullimageview
            //anchors.verticalCenter: parent.verticalCenter
            width: 1826
            height: 284
            fillMode: Image.PreserveAspectFit
            visible: height > 284
            Behavior on height { NumberAnimation {} }
        }

        MouseArea
        {
            anchors.fill: parent
            enabled: zoomin

            onClicked:
            {
                console.log("clicked")
                fullimageview.height = 284
                zoomin = false
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
        }
    }
}


