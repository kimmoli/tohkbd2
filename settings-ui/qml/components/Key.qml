import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea
{
    id: ma
    property string label: ""
    property string symLabel: ""

    property bool changed: false

    enabled: label.length > 0

    onClicked:
    {
        console.log(label)

        var dlg = pageStack.push(Qt.resolvedUrl("../pages/CustomizeKey.qml"),
                                 {
                                     "label": label,
                                     "symLabel": symLabel
                                 } )

        dlg.accepted.connect(function()
        {
            console.log("Change of " + label + " accepted.")
            changed = true
        })
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
        color: (ma.pressed ? Theme.highlightColor : (changed ? "red" : "transparent"))
        opacity: ma.pressed ? 0.6 : 0.3
        radius: 10
    }
}
