/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page
{
    id: page

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    KeyboardHandler
    {
        id: kbdif
        onKeyUpPressed: flick.flick(0, -1000)
        onKeyDownPressed: flick.flick(0, 1000)
        onKeyBackspacePressed: pageStack.pop()
    }

    SilicaFlickable
    {
        id: flick
        anchors.fill: parent

        contentHeight: column.height

        PullDownMenu
        {
            MenuItem
            {
                text: qsTrId("reset-to-defaults")
                onClicked:
                {
                    settingsui.setSettingsToDefault()
                    pageStack.pop()
                }
            }
        }

        Column
        {
            id: column

            width: page.width

            PageHeader
            {
                title: qsTrId("general-settings")
            }

            SectionHeader
            {
                //: Section header for backlight settings
                //% "Backlight"
                text: qsTrId("backlight-sect-header")
            }
            TextSwitch
            {
                id: alwaysOn
                //: Backlight always on switch text
                //% "Always on"
                text: qsTrId("bg-always-on-sw")
                //: Backlight always on description
                //% "Backlight is always on when keyboard attached and phone's display is on"
                description: qsTrId("bg-always-on-desc")
                onCheckedChanged: settingsui.setSettingInt("forceBacklightOn", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["forceBacklightOn"]
            }
            TextSwitch
            {
                id: automatic
                //: Backlight automatic switch text
                //% "Automatic"
                text: qsTrId("bg-automatic-sw")
                //: Backlight automatic description
                //% "Automatic backlight enable or always off"
                description: qsTrId("bg-automatic-desc")
                onCheckedChanged: settingsui.setSettingInt("backlightEnabled", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["backlightEnabled"]
                enabled: !alwaysOn.checked
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                //: Backlight timeout slider name
                //% "Backlight timeout"
                label: qsTrId("bg-timeout-slider")
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
                //: Backlight brightness threshold slider
                //% "Brightness threshold"
                label: qsTrId("bg-brightness-slider")
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
                //: Section header for orientation settings
                //% "Orientation"
                text: qsTrId("orientation-sect-header")
            }
            TextSwitch
            {
                //: Force landsacep switch text
                //% "Force Landscape"
                text: qsTrId("orientation-force-landscape-sw")
                //: Force landsacep switch description
                //% "Force landscape orientation when keyboard attached"
                description: qsTrId("orientation-force-landscape-desc")
                onCheckedChanged: settingsui.setSettingInt("forceLandscapeOrientation", checked ? 1 : 0)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["forceLandscapeOrientation"]
            }

            SectionHeader
            {
                //: Section header for repeat settings
                //% "Repeat"
                text: qsTrId("repeat-sect-header")
            }
            Slider
            {
                width: parent.width - 2*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                //: Keyboard repeat start delay slider
                //% "Repeat start delay"
                label: qsTrId("repeat-delay-slider")
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
                //: Keyboard repeat rate slider
                //% "Repeat rate"
                label: qsTrId("repeat-rate-slider")
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
                //: Placeholder text for textfield to test repeat settings
                //% "Test here"
                placeholderText: qsTrId("test-here")
                onFocusChanged:
                {
                    /* Restore focus back to keyboard handler */
                    if (!focus)
                        kbdif.focus = true
                }
            }

            SectionHeader
            {
                //: Section header for sticky and locking settings
                //% "Sticky and locking modifier keys"
                text: qsTrId("sticky-sect-header")
            }
            Label
            {
                //: Description text for sticky and locking modifier keys
                //% "Sticky modifiers will toggle when pressed once and released after pressing any other key. Locking modifier will lock on double-press and released on third. In both modes you can also use them as normal modifier keys"
                text: qsTrId("sticky-desc")
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
                    //% "Sticky Shift"
                    text: qsTrId("sticky-shift")
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
                    //% "Locking Shift"
                    text: qsTrId("locking-shift")
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
                    //% "Sticky Ctrl"
                    text: qsTrId("sticky-ctrl")
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
                    //% "Locking Ctrl"
                    text: qsTrId("locking-ctrl")
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
                    //% "Sticky Alt"
                    text: qsTrId("sticky-alt")
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
                    //% "Locking Alt"
                    text: qsTrId("locking-alt")
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
                    //% "Sticky Sym"
                    text: qsTrId("sticky-sym")
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
                    //% "Locking Sym"
                    text: qsTrId("locking-sym")
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

