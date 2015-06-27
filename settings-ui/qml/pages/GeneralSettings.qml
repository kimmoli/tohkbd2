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
                label: qsTr("Backlight timeout")
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
                label: qsTr("Repeat start delay")
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
                label: qsTr("Repeat rate")
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
                text: qsTr("Sticky and locking modifier keys")
            }
            Label
            {
                text: qsTr("Sticky modifiers will toggle when pressed once and released after pressing any other key. Locking modifier will lock on double-press and released on third. In both modes you can also use them as normal modifier keys")
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                width: parent.width - 4*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row
            {
                width: parent.width - 2*Theme.paddingLarge
                TextSwitch
                {
                    id: stickyShift
                    text: qsTr("Sticky Shift")
                    onCheckedChanged:
                    {
                        if (checked)
                            lockingShift.checked = false
                        settingsui.setSettingInt("stickyShiftEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["stickyShiftEnabled"]
                }
                TextSwitch
                {
                    id: lockingShift
                    text: qsTr("Locking Shift")
                    onCheckedChanged:
                    {
                        if (checked)
                            stickyShift.checked = false
                        settingsui.setSettingInt("lockingShiftEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["lockingShiftEnabled"]
                }
            }
            Row
            {
                width: parent.width - 2*Theme.paddingLarge
                TextSwitch
                {
                    id: stickyCtrl
                    text: qsTr("Sticky Ctrl")
                    onCheckedChanged:
                    {
                        if (checked)
                            lockingCtrl.checked = false
                        settingsui.setSettingInt("stickyCtrlEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["stickyCtrlEnabled"]
                }
                TextSwitch
                {
                    id: lockingCtrl
                    text: qsTr("Locking Ctrl")
                    onCheckedChanged:
                    {
                        if (checked)
                            stickyCtrl.checked = false
                        settingsui.setSettingInt("lockingCtrlEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["lockingCtrlEnabled"]
                }
            }
            Row
            {
                width: parent.width - 2*Theme.paddingLarge
                TextSwitch
                {
                    id: stickyAlt
                    text: qsTr("Sticky Alt")
                    onCheckedChanged:
                    {
                        if (checked)
                            lockingAlt.checked = false
                        settingsui.setSettingInt("stickyAltEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["stickyAltEnabled"]
                }
                TextSwitch
                {
                    id: lockingAlt
                    text: qsTr("Locking Alt")
                    onCheckedChanged:
                    {
                        if (checked)
                            stickyAlt.checked = false;
                        settingsui.setSettingInt("lockingAltEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["lockingAltEnabled"]
                }
            }
            Row
            {
                width: parent.width - 2*Theme.paddingLarge
                TextSwitch
                {
                    id: stickySym
                    text: qsTr("Sticky Sym")
                    onCheckedChanged:
                    {
                        if (checked)
                            lockingSym.checked = false
                        settingsui.setSettingInt("stickySymEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["stickySymEnabled"]
                }
                TextSwitch
                {
                    id: lockingSym
                    text: qsTr("Locking Sym")
                    onCheckedChanged:
                    {
                        if (checked)
                            stickySym.checked = false
                        settingsui.setSettingInt("lockingSymEnabled", checked ? 1 : 0)
                    }
                    width: parent.width/2
                    Component.onCompleted: checked = settings["lockingSymEnabled"]
                }
            }
        }
    }
}

