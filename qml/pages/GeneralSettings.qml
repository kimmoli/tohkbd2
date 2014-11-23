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
                title: "General settings"
            }

            SectionHeader
            {
                text: "Backlight"
            }
            TextSwitch
            {
                id: autoBacklightSwitch
                text: "Automatic backlight"
                x: Theme.paddingLarge
                checked: true
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: "Timeout"
                minimumValue: 100
                maximumValue: 5000
                value: settings["backlightTimeout"]
                valueText: value + " ms"
                stepSize: 100
                enabled: autoBacklightSwitch.checked

                property bool wasChanged: false
                onValueChanged: wasChanged = true
                onReleased:
                {
                    if (wasChanged)
                    {
                        wasChanged = false
                        settingsui.setSettingInt("backlightTimeout", value)
                    }
                }
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: "Brightness threshold"
                minimumValue: 1
                maximumValue: 50
                value: 5
                valueText: value + " lux"
                stepSize: 1
                enabled: autoBacklightSwitch.checked
            }

            SectionHeader
            {
                text: "Repeat"
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: "Delay"
                minimumValue: 50
                maximumValue: 500
                value: 250
                valueText: value + " ms"
                stepSize: 10
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: "Rate"
                minimumValue: 25
                maximumValue: 100
                value: 25
                valueText: value + " ms"
                stepSize: 1
            }
        }
    }
}

