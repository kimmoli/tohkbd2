import QtQuick 2.0
import Sailfish.Silica 1.0

Item
{
    id: kbdif
    anchors.fill: parent
    focus: true

    property int upDownItemCount: 0
    property int leftRightItemCount: 0
    property int upDownSelection: -1
    property int leftRightSelection: -1

    signal keyUpPressed
    signal keyDownPressed
    signal keyLeftPressed
    signal keyRightPressed
    signal keyEnterPressed
    signal keyBackspacePressed

    Keys.onPressed:
    {
        if (event.key === Qt.Key_Down)
        {
            if (++upDownSelection >= upDownItemCount)
                upDownSelection = 0
            keyUpPressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Up)
        {
            if (upDownSelection <= 0)
                upDownSelection = upDownItemCount
            upDownSelection--
            keyDownPressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Right)
        {
            if (++leftRightSelection >= leftRightItemCount)
                leftRightSelection = 0
            keyRightPressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Left)
        {
            if (leftRightSelection <= 0)
                leftRightSelection = leftRightItemCount
            leftRightSelection--
            keyLeftPressed()
            event.accepted = true
        }
        else if (event.key === Qt.Key_Return)
        {
            if (upDownItemCount > 0 && upDownSelection >= 0 && upDownSelection < upDownItemCount)
            {
                keyEnterPressed()
            }
            if (leftRightItemCount > 0 && leftRightSelection >= 0 && leftRightSelection < leftRightItemCount)
            {
                keyEnterPressed()
            }
            event.accepted = true
        }
        else if (event.key === Qt.Key_Backspace)
        {
            keyBackspacePressed()
            event.accepted = true
        }
    }

    /* When using SilicaFlickable and Repeater in Column, this moves flickable so selected item should be visible.
     * For softer movement, add e.g. following to SilicaFlickable
     * Behavior on contentY { NumberAnimation { duration: 200 } }
     */
    function flickRepeaterMover(flick, repeater, pageheader, page)
    {
        var itm = repeater.itemAt(upDownSelection)

        if ((itm.y + itm.height) - flick.contentY > page.height)
            flick.contentY = itm.y - ((page.height / itm.height) -1 ) * itm.height

        if ((itm.y < flick.contentY))
            flick.contentY = itm.y - pageheader.height
    }

}
