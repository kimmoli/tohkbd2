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
    clip: true

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
                enabled: false // !zoomin && view.currentIndex == index
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
        text: layoutsmodel.count ? layoutsmodel.get(view.currentIndex).layoutname : ""
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeLarge
        opacity: fullimageview.visible ? 0 : 1
        Behavior on opacity { NumberAnimation {} }
    }
    Text
    {
        id: layoutlabelActive
        anchors.bottom: layoutlabel.top
        anchors.bottomMargin: Theme.paddingSmall
        anchors.horizontalCenter: page.horizontalCenter
        //: label showing this layout is currently selected
        //% "Active"
        text: qsTrId("active-layout")
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeMedium
        visible: layoutlabel.text === settings["masterLayout"]
        opacity: fullimageview.visible ? 0 : 1
        Behavior on opacity { NumberAnimation {} }
    }
    Button
    {
        id: layoutSelectButton
        enabled: layoutsmodel.count ? (layoutsmodel.get(view.currentIndex).implemented && opacity == 1) : false
        //: Button text for button to select this as active LayoutItem
        //% "Select"
        text: qsTrId("select-layout")
        anchors.top: view.bottom
        anchors.topMargin: Theme.paddingLarge
        anchors.horizontalCenter: page.horizontalCenter
        opacity: fullimageview.visible ? 0 : 1
        Behavior on opacity { NumberAnimation {} }

        onClicked:
        {
            settingsui.setSettingString("masterLayout", layoutsmodel.get(view.currentIndex).layoutname)
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
                        symLabel: sym
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
                        symLabel: sym
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
                        symLabel: sym
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
                        symLabel: sym
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
                        symLabel: sym
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
            row1.append({name:"Tab", sym:"Esc", size: 2})
            row1.append({name:"1",   sym:"",    size: 1})
            row1.append({name:"2",   sym:"",    size: 1})
            row1.append({name:"3",   sym:"",    size: 1})
            row1.append({name:"4",   sym:"",    size: 1})
            row1.append({name:"5",   sym:"",    size: 1})
            row1.append({name:"6",   sym:"",    size: 1})
            row1.append({name:"7",   sym:"",    size: 1})
            row1.append({name:"8",   sym:"",    size: 1})
            row1.append({name:"9",   sym:"",    size: 1})
            row1.append({name:"0",   sym:"",    size: 1})
            row1.append({name:"-",   sym:"",    size: 1})
            row1.append({name:"=",   sym:"",    size: 1})
            row1.append({name:"Backspace", sym:"", size: 2})
        }
    }
    ListModel
    {
        id: row2
        Component.onCompleted:
        {
            row2.append({name:"Del", sym:"",    size: 1})
            row2.append({name:"Up",  sym:"PgUp", size: 1})
            row2.append({name:"Ins", sym:"",    size: 1})
            row2.append({name:"Q",   sym:"~",   size: 1})
            row2.append({name:"W",   sym:"#",   size: 1})
            row2.append({name:"E",   sym:"$",   size: 1})
            row2.append({name:"R",   sym:"%",   size: 1})
            row2.append({name:"T",   sym:"^",   size: 1})
            row2.append({name:"Y",   sym:"&",   size: 1})
            row2.append({name:"U",   sym:"*",   size: 1})
            row2.append({name:"I",   sym:"{",   size: 1})
            row2.append({name:"O",   sym:"}",   size: 1})
            row2.append({name:"P",   sym:"_",   size: 1})
            row2.append({name:"Å",   sym:"+",   size: 1})
            row2.append({name:"Right Ctrl", sym:"", size: 2})
        }
    }
    ListModel
    {
        id: row3
        Component.onCompleted:
        {
            row3.append({name:"Left",  sym:"",    size: 1})
            row3.append({name:"",      sym:"",    size: 1})
            row3.append({name:"Right", sym:"",    size: 1})
            row3.append({name:"A",     sym:";",   size: 1})
            row3.append({name:"S",     sym:"'",   size: 1})
            row3.append({name:"D",     sym:"£",   size: 1})
            row3.append({name:"F",     sym:"",    size: 1})
            row3.append({name:"G",     sym:"`",   size: 1})
            row3.append({name:"H",     sym:"",    size: 1})
            row3.append({name:"J",     sym:"",    size: 1})
            row3.append({name:"K",     sym:"[",   size: 1})
            row3.append({name:"L",     sym:"}",   size: 1})
            row3.append({name:"Ö",     sym:"(",   size: 1})
            row3.append({name:"Ä",     sym:")",   size: 1})
            row3.append({name:"Alt",   sym:"",    size: 2})
        }
    }
    ListModel
    {
        id: row4
        Component.onCompleted:
        {
            row4.append({name:"Home", sym:"",   size: 1})
            row4.append({name:"Down", sym:"PgDn", size: 1})
            row4.append({name:"End",  sym:"",   size: 1})
            row4.append({name:"Z",    sym:":",  size: 1})
            row4.append({name:"X",    sym:"\"", size: 1})
            row4.append({name:"C",    sym:"€",  size: 1})
            row4.append({name:"V",    sym:"/",  size: 1})
            row4.append({name:"B",    sym:"|",  size: 1})
            row4.append({name:"N",    sym:"\\", size: 1})
            row4.append({name:"M",    sym:"µ",  size: 1})
            row4.append({name:"?",    sym:"",   size: 1})
            row4.append({name:"!",    sym:"",   size: 1})
            row4.append({name:",",    sym:"<",  size: 1})
            row4.append({name:".",    sym:">",  size: 1})
            row4.append({name:"Enter", sym:"",  size: 2})
        }
    }
    ListModel
    {
        id: row5
        Component.onCompleted:
        {
            row5.append({name:"Left Sym",    sym:"",   size: 2})
            row5.append({name:"Left Ctrl",   sym:"",   size: 1})
            row5.append({name:"Left Shift",  sym:"",   size: 2})
            row5.append({name:"Space",       sym:"",   size: 4})
            row5.append({name:"Right Shift", sym:"",   size: 2})
            row5.append({name:"@",           sym:"",   size: 1})
            row5.append({name:"Right Sym",   sym:"",   size: 2})
            row5.append({name:"Enter",       sym:"",   size: 2})
        }
    }

    ListModel
    {
        id: layoutsmodel

        Component.onCompleted:
        {
            layoutsmodel.append({"imagesource":"../images/image-keyboard-scandic.png", "layoutname":"Scandic", "implemented": true })
            layoutsmodel.append({"imagesource":"../images/image-keyboard-qwerty.png", "layoutname":"QWERTY", "implemented": false })
            layoutsmodel.append({"imagesource":"../images/image-keyboard-qwertz.png", "layoutname":"QWERTZ", "implemented": true })
            layoutsmodel.append({"imagesource":"../images/image-keyboard-azerty.png", "layoutname":"AZERTY", "implemented": true })
            layoutsmodel.append({"imagesource":"../images/image-keyboard-cyrillic.png", "layoutname":"Cyrillic", "implemented": false })
            layoutsmodel.append({"imagesource":"../images/image-keyboard-stealth.png", "layoutname":"Stealth", "implemented": false })
        }
    }

}


