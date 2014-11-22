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

            PageHeader
            {
                title: "Keyboard layout"
            }
            Repeater
            {
                model: layoutslist

                ListItem
                {
                    id: listItem
                    height: Theme.itemSizeSmall

                    Switch
                    {
                        id: sw
                        checked: selected
                        anchors.verticalCenter: parent.verticalCenter
                        x: Theme.paddingLarge
                    }
                    Label
                    {
                        text: labelId
                        anchors.left: sw.right
                        anchors.leftMargin: Theme.paddingSmall
                        anchors.verticalCenter: parent.verticalCenter
                        color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }

                    onClicked: enableLayout(index)
                }
            }
        }
    }

    ListModel
    {
        id: layoutslist

        Component.onCompleted:
        {
            layoutslist.append({"labelId":"QWERTY",   "selected":true})
            layoutslist.append({"labelId":"QWERTZ",   "selected":false})
            layoutslist.append({"labelId":"AZERTY",   "selected":false})
            layoutslist.append({"labelId":"Scandic",  "selected":false})
            layoutslist.append({"labelId":"Cyrillic", "selected":false})
        }
    }

    function enableLayout(index)
    {
        var i
        for (i=0 ; i<layoutslist.count ; i++)
            layoutslist.setProperty(i, "selected", i === index)

        // TODO: Send selected layout to daemon
    }
}
