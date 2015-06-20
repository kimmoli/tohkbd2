import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea
{
    id: ma
    property string label: ""
    enabled: label.length > 0

    onClicked:
    {
        console.log(label)
        pageStack.push(Qt.resolvedUrl("../pages/CustomizeKey.qml"), { "label": label } )
    }
    onPressAndHold:
    {
        console.log("clicked")
        fullimageview.height = 284
        zoomin = false
    }

    Rectangle
    {
        anchors.fill: parent
        color: Theme.highlightColor
        opacity: ma.pressed ? 0.6 : 0.0
    }
}
