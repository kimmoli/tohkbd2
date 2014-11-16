/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground
{
    CoverPlaceholder
    {
        text: "Tohkbd2-settings-u"
        icon.source: "/usr/share/icons/hicolor/86x86/apps/tohkbd2-settings-u.png"
    }

    CoverActionList
    {
        CoverAction
        {
            iconSource: coverActionLeftIcon
            onTriggered: coverActionLeft()
        }

        CoverAction
        {
            iconSource: coverActionRightIcon
            onTriggered: coverActionRight()
        }
    }
}


