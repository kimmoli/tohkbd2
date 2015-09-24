/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import "../components"

Page
{
    id: page

    function getIndexOfCurrentLayout()
    {
        for (var i = 0; i < layoutsModel.count; ++i)
        {
            if (layoutsModel.get(i).name === settings["physicalLayout"])
            {
                return i
            }
        }
        return -1
    }

    property int indexOfCurrentLayout: -1

    Component.onCompleted:
    {
        indexOfCurrentLayout = getIndexOfCurrentLayout()
        listView.positionViewAtIndex(indexOfCurrentLayout, ListView.Center)
        kbdif.upDownSelection = indexOfCurrentLayout
    }

    KeyboardHandler
    {
        id: kbdif
        upDownItemCount: layoutsModel.count
        onKeyUpPressed: listView.positionViewAtIndex(upDownSelection, ListView.Contain)
        onKeyDownPressed: listView.positionViewAtIndex(upDownSelection, ListView.Contain)

        onKeyEnterPressed:
        {
            keymapLayout.value = layoutsModel.get(upDownSelection).key
            pageStack.pop()
        }
        onKeyBackspacePressed: pageStack.pop()

        Connections
        {
            target: page
            onOrientationTransitionRunningChanged: if (!orientationTransitionRunning) listView.positionViewAtIndex(kbdif.upDownSelection, ListView.Contain)
        }

    }

    SilicaListView
    {
        id: listView
        model: layoutsModel
        anchors.fill: parent

        VerticalScrollDecorator {}

        header: PageHeader
        {
            title: qsTrId("kbd-layout")
        }

        delegate: BackgroundItem
        {
            id: delegate
            highlighted: index === kbdif.upDownSelection
            height: Theme.itemSizeSmall
            onDownChanged: kbdif.upDownSelection = index

            Label
            {
                x: Theme.paddingLarge
                text: name
                anchors.verticalCenter: parent.verticalCenter
                color: (delegate.highlighted || index == indexOfCurrentLayout) ? Theme.highlightColor : Theme.primaryColor
                opacity: supported ? 1.0 : 0.65
            }
            onClicked:
            {
                keymapLayout.value = key
                pageStack.pop()
            }
        }
    }

    ConfigurationValue
    {
        id: keymapLayout
        key: "/desktop/lipstick-jolla-home/layout"
        onValueChanged: indexOfCurrentLayout = getIndexOfCurrentLayout()
    }
}
