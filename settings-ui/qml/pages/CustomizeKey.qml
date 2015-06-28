/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    property string label: "value"
    property string symLabel: "value"

    onDone:
    {
        if (result === DialogResult.Accepted)
        {
        }
    }

    SilicaFlickable
    {
        anchors.fill: parent
        contentHeight: col.height
        Column
        {
            id: col
            width: parent.width
            spacing: Theme.paddingLarge
            PageHeader
            {
                //: page header for key customizer, shows label of the key
                //% "Customize %1 -key"
                title: qsTrId("customize-key-header").arg(label)
            }
            Label
            {
                //: warning text shown on customizer page
                //% "You are about to change behaviour of this key"
                text: qsTrId("customize-key-warning")
                color: Theme.primaryColor
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label
            {
                //: Text shon above the key without any modifiers, e.g. A
                //% "Plain key"
                text: qsTrId("plain-key")
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                text: label
                font.bold: true
                font.pixelSize: Theme.fontSizeLarge
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle
                {
                    id: plainKeyRect
                    anchors.centerIn: parent
                    height: parent.height + 10
                    width: Math.max(height, parent.width + 16)
                    border.color: Theme.primaryColor
                    border.width: 3
                    radius: 5
                    color: "transparent"
                }
                OpacityRampEffect
                {
                    direction: OpacityRamp.LeftToRight
                    slope: 1.0
                    offset: 0.33
                    sourceItem: plainKeyRect
                }
            }
            Label
            {
                //: Text shon above the key wit sym modifier, e.g. ;
                //% "Key with Sym"
                text: qsTrId("with-sym-key")
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Label
            {
                //: This text is shown instead of key label (e.g. A) if there is nothing assigned to this key
                //% "None"
                text: symLabel.length > 0 ? symLabel : qsTrId("none-key")
                font.bold: true
                font.pixelSize: Theme.fontSizeLarge
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle
                {
                    id: symKeyRect
                    visible: symLabel.length > 0
                    anchors.centerIn: parent
                    height: parent.height + 10
                    width: Math.max(height, parent.width + 16)
                    border.color: Theme.primaryColor
                    border.width: 3
                    radius: 5
                    color: "transparent"
                }
                OpacityRampEffect
                {
                    direction: OpacityRamp.LeftToRight
                    slope: 1.0
                    offset: 0.33
                    sourceItem: symKeyRect
                }
            }
        }
    }
}

