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
                title: qsTr("General settings")
            }

            SectionHeader
            {
                text: qsTr("Backlight")
            }
            TextSwitch
            {
                id: alwaysOn
                text: qsTr("Always on")
                description: qsTr("Backlight is always on when keyboard attached and phone's display is on")
                onCheckedChanged: settingsui.setSettingInt("forceBacklightOn", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["forceBacklightOn"]
            }
            TextSwitch
            {
                id: automatic
                text: qsTr("Automatic")
                description: qsTr("Automatic backlight enable or always off")
                onCheckedChanged: settingsui.setSettingInt("backlightEnabled", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["backlightEnabled"]
                enabled: !alwaysOn.checked
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: qsTr("Timeout")
                minimumValue: 100
                maximumValue: 5000
                value: settings["backlightTimeout"]
                valueText: value + " ms"
                stepSize: 100
                enabled: !alwaysOn.checked && automatic.checked
                opacity: enabled ? 1.0 : 0.4

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
                label: qsTr("Brightness threshold")
                minimumValue: 1
                maximumValue: 50
                value: settings["backlightLuxThreshold"]
                valueText: value + " lux"
                stepSize: 1
                enabled: !alwaysOn.checked && automatic.checked
                opacity: enabled ? 1.0 : 0.4

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
                text: qsTr("Orientation")
            }
            TextSwitch
            {
                text: qsTr("Force Landscape")
                description: qsTr("Force landscape orientation when keyboard attached")
                onCheckedChanged: settingsui.setSettingInt("forceLandscapeOrientation", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["forceLandscapeOrientation"]
            }

            SectionHeader
            {
                text: qsTr("Repeat")
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                label: qsTr("Delay")
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
                label: qsTr("Rate")
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
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                placeholderText: qsTr("Test here")
            }

            SectionHeader
            {
                text: qsTr("Sticky modifiers")
            }
            Label
            {
                text: qsTr("Sticky modifiers will toggle when pressed, they also work as normal modifier keys")
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                width: parent.width - 4*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextSwitch
            {
                text: qsTr("Sticky Ctrl")
                onCheckedChanged: settingsui.setSettingInt("stickyCtrlEnabled", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["stickyCtrlEnabled"]
            }
            TextSwitch
            {
                text: qsTr("Sticky Alt")
                onCheckedChanged: settingsui.setSettingInt("stickyAltEnabled", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["stickyAltEnabled"]
            }
            TextSwitch
            {
                text: qsTr("Sticky Sym")
                onCheckedChanged: settingsui.setSettingInt("stickySymEnabled", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["stickySymEnabled"]
            }

        }
    }
}

