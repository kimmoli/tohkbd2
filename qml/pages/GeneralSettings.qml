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
                value: settings["backlightLuxThreshold"]
                valueText: value + " lux"
                stepSize: 1
                enabled: autoBacklightSwitch.checked

                property bool wasChanged: false
                onValueChanged: wasChanged = true
                onReleased:
                {
                    if (wasChanged)
                    {
                        wasChanged = false
                        settingsui.setSettingInt("backlightLuxThreshold", value)
                    }
                }
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
                value: settings["keyRepeatDelay"]
                valueText: value + " ms"
                stepSize: 10

                property bool wasChanged: false
                onValueChanged: wasChanged = true
                onReleased:
                {
                    if (wasChanged)
                    {
                        wasChanged = false
                        settingsui.setSettingInt("keyRepeatDelay", value)
                    }
                }
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: "Rate"
                minimumValue: 25
                maximumValue: 100
                value: settings["keyRepeatRate"]
                valueText: value + " ms"
                stepSize: 1

                property bool wasChanged: false
                onValueChanged: wasChanged = true
                onReleased:
                {
                    if (wasChanged)
                    {
                        wasChanged = false
                        settingsui.setSettingInt("keyRepeatRate", value)
                    }
                }
            }
            TextField
            {
                width: parent.width - Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                placeholderText: "Test here"
            }
        }
    }
}

