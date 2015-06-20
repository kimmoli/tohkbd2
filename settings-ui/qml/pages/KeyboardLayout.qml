/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

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
        height: 290
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
        anchors.bottom: view.top
        anchors.bottomMargin: Theme.paddingLarge
        anchors.horizontalCenter: page.horizontalCenter
        text: layoutsmodel.count < 5 ? "" : layoutsmodel.get(view.currentIndex).layoutname
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeLarge
        opacity: fullimageview.visible ? 0 : 1
        Behavior on opacity { NumberAnimation {} }
    }
    Button
    {
        id: layoutSelectButton
        text: qsTr("Select")
        anchors.top: view.bottom
        anchors.topMargin: Theme.paddingLarge
        anchors.horizontalCenter: page.horizontalCenter
        opacity: fullimageview.visible ? 0 : 1
        Behavior on opacity { NumberAnimation {} }
        enabled: opacity == 1

        onClicked:
        {
            pageStack.pop()
        }
    }

    SilicaFlickable
    {
        id: fullimageviewFlickable
        visible: fullimageview.height > 290
        height: fullimageview.height
        width: parent.width

        contentHeight: fullimageview.height
        contentWidth: fullimageview.width
        anchors.verticalCenter: parent.verticalCenter

        Image
        {
            id: fullimageview
            //anchors.verticalCenter: parent.verticalCenter
            width: 1787
            height: 290
            fillMode: Image.PreserveAspectFit
            visible: height > 290
            Behavior on height { NumberAnimation {} }
        }

        Column
        {
            anchors.left: fullimageview.left
            anchors.top: fullimageview.top

            Row
            {
                width: fullimageview.width
                height: (fullimageview.height - spacer.height)/5

                Repeater
                {
                    model: row1
                    delegate: Key {
                        label: name
                        width: size * (fullimageview.width/16)
                        height: fullimageview.height/5
                    }
                }
            }
            Item
            {
                id: spacer
                height: 21
                width: 1
            }
            Row
            {
                width: fullimageview.width
                height: (fullimageview.height - spacer.height)/5

                Repeater
                {
                    model: row2
                    delegate: Key {
                        label: name
                        width: size * (fullimageview.width/16)
                        height: fullimageview.height/5
                    }
                }
            }
            Row
            {
                width: fullimageview.width
                height: (fullimageview.height - spacer.height)/5

                Repeater
                {
                    model: row3
                    delegate: Key {
                        label: name
                        width: size * (fullimageview.width/16)
                        height: fullimageview.height/5
                    }
                }
            }
            Row
            {
                width: fullimageview.width
                height: (fullimageview.height - spacer.height)/5

                Repeater
                {
                    model: row4
                    delegate: Key {
                        label: name
                        width: size * (fullimageview.width/16)
                        height: fullimageview.height/5
                    }
                }
            }
            Row
            {
                width: fullimageview.width
                height: (fullimageview.height - spacer.height)/5

                Repeater
                {
                    model: row5
                    delegate: Key {
                        label: name
                        width: size * (fullimageview.width/16)
                        height: fullimageview.height/5
                    }
                }
            }

        }
    }

    ListModel
    {
        id: row1
        Component.onCompleted:
        {
            row1.append({"name":"Tab", "size": 2})
            row1.append({"name":"1", "size": 1})
            row1.append({"name":"2", "size": 1})
            row1.append({"name":"3", "size": 1})
            row1.append({"name":"4", "size": 1})
            row1.append({"name":"5", "size": 1})
            row1.append({"name":"6", "size": 1})
            row1.append({"name":"7", "size": 1})
            row1.append({"name":"8", "size": 1})
            row1.append({"name":"9", "size": 1})
            row1.append({"name":"0", "size": 1})
            row1.append({"name":"-", "size": 1})
            row1.append({"name":"=", "size": 1})
            row1.append({"name":"Backspace", "size": 2})
        }
    }
    ListModel
    {
        id: row2
        Component.onCompleted:
        {
            row2.append({"name":"Del", "size": 1})
            row2.append({"name":"Up", "size": 1})
            row2.append({"name":"Ins", "size": 1})
            row2.append({"name":"Q", "size": 1})
            row2.append({"name":"W", "size": 1})
            row2.append({"name":"E", "size": 1})
            row2.append({"name":"R", "size": 1})
            row2.append({"name":"T", "size": 1})
            row2.append({"name":"Y", "size": 1})
            row2.append({"name":"U", "size": 1})
            row2.append({"name":"I", "size": 1})
            row2.append({"name":"O", "size": 1})
            row2.append({"name":"P", "size": 1})
            row2.append({"name":"Å", "size": 1})
            row2.append({"name":"Right Ctrl", "size": 2})
        }
    }
    ListModel
    {
        id: row3
        Component.onCompleted:
        {
            row3.append({"name":"Left", "size": 1})
            row3.append({"name":"", "size": 1})
            row3.append({"name":"Right", "size": 1})
            row3.append({"name":"A", "size": 1})
            row3.append({"name":"S", "size": 1})
            row3.append({"name":"D", "size": 1})
            row3.append({"name":"F", "size": 1})
            row3.append({"name":"G", "size": 1})
            row3.append({"name":"H", "size": 1})
            row3.append({"name":"J", "size": 1})
            row3.append({"name":"K", "size": 1})
            row3.append({"name":"L", "size": 1})
            row3.append({"name":"Ö", "size": 1})
            row3.append({"name":"Ä", "size": 1})
            row3.append({"name":"Alt", "size": 2})
        }
    }
    ListModel
    {
        id: row4
        Component.onCompleted:
        {
            row4.append({"name":"Home", "size": 1})
            row4.append({"name":"Down", "size": 1})
            row4.append({"name":"End", "size": 1})
            row4.append({"name":"Z", "size": 1})
            row4.append({"name":"X", "size": 1})
            row4.append({"name":"C", "size": 1})
            row4.append({"name":"V", "size": 1})
            row4.append({"name":"B", "size": 1})
            row4.append({"name":"N", "size": 1})
            row4.append({"name":"M", "size": 1})
            row4.append({"name":"?", "size": 1})
            row4.append({"name":"!", "size": 1})
            row4.append({"name":",", "size": 1})
            row4.append({"name":".", "size": 1})
            row4.append({"name":"Enter", "size": 2})
        }
    }
    ListModel
    {
        id: row5
        Component.onCompleted:
        {
            row5.append({"name":"Left Sym", "size": 2})
            row5.append({"name":"Left Ctrl", "size": 1})
            row5.append({"name":"Left Shift", "size": 2})
            row5.append({"name":"Space", "size": 4})
            row5.append({"name":"Right Shift", "size": 2})
            row5.append({"name":"@", "size": 1})
            row5.append({"name":"Right Sym", "size": 2})
            row5.append({"name":"Enter", "size": 2})
        }
    }

    ListModel
    {
        id: layoutsmodel

        Component.onCompleted:
        {
            layoutsmodel.append({"imagesource":"../images/image-keyboard-scandic.png", "layoutname":"Scandic"})
            layoutsmodel.append({"imagesource":"../images/image-keyboard-qwerty.png", "layoutname":"QWERTY"})
            layoutsmodel.append({"imagesource":"../images/image-keyboard-qwertz.png", "layoutname":"QWERTZ"})
            layoutsmodel.append({"imagesource":"../images/image-keyboard-azerty.png", "layoutname":"AZERTY"})
            layoutsmodel.append({"imagesource":"../images/image-keyboard-cyrillic.png", "layoutname":"Cyrillic"})
            layoutsmodel.append({"imagesource":"../images/image-keyboard-stealth.png", "layoutname":"Stealth"})
        }
    }

}


