/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import "../components"

Page
{
    id: page

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
                onCheckedChanged: if (checked !== settings["forceBacklightOn"]) settingsui.setSetting("forceBacklightOn", checked)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted:
                {
                    checked = settings["forceBacklightOn"]
                    /* Update to restore if overridden with key-combo Sym+Home */
                    if (!checked && !settings["forceBacklightOn"])
                        settingsui.setSetting("forceBacklightOn", false)
                }
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
                onCheckedChanged: if (checked !== settings["backlightEnabled"]) settingsui.setSetting("backlightEnabled", checked)
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
                maximumValue: 30000
                value: settings["backlightTimeout"]
                valueText: Number(value/1000).toFixed(1) + " s"
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
                        settingsui.setSetting("backlightTimeout", value)
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
                        settingsui.setSetting("backlightLuxThreshold", value)
                    }
                }
            }

            SectionHeader
            {
                //: Section header for display related settings
                //% "Display"
                text: qsTrId("orientation-sect-header")
            }
            TextSwitch
            {
                //: Force landscape switch text
                //% "Force Landscape"
                text: qsTrId("orientation-force-landscape-sw")
                //: Force landscape switch description
                //% "Force landscape orientation when keyboard attached"
                description: qsTrId("orientation-force-landscape-desc")
                onCheckedChanged: settingsui.setSetting("forceLandscapeOrientation", checked)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["forceLandscapeOrientation"]
            }
            TextSwitch
            {
                //: Keep display on when connected switch text
                //% "Display on when connected"
                text: qsTrId("keep-display-on-when-connected-sw")
                //: Keep display on when connected switch description
                //% "Keep display on when keyboard is connected"
                description: qsTrId("keep-display-on-when-connected-desc")
                onCheckedChanged: settingsui.setSetting("keepDisplayOnWhenConnected", checked)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["keepDisplayOnWhenConnected"]
            }
            TextSwitch
            {
                //: Display off when removed switch text
                //% "Display off when removed"
                text: qsTrId("turn-display-off-when-removed-sw")
                //: Display off when removed switch description
                //% "Turn display off when keyboard is removed"
                description: qsTrId("turn-display-off-when-removed-desc")
                onCheckedChanged: settingsui.setSetting("turnDisplayOffWhenRemoved", checked)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["turnDisplayOffWhenRemoved"]
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
                        settingsui.setSetting("keyRepeatDelay", value)
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
                        settingsui.setSetting("keyRepeatRate", value)
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
                //% "In Sticky mode, modifier key will stay on after pressed once and released after pressing again or any other key. In Lock mode modifier key will lock on double-press and released on third. In Cycle mode Sticky and Lock modes are both active, after first press is Sticky and second press is Lock. In all modes you can also use them as normal modifier keys."
                text: qsTrId("sticky-desc")
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                width: parent.width - 4*Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Repeater
            {
                model: modifiers

                ComboBox
                {
                    width: parent.width
                    label: "   " + combolabel + " "
                    menu: ContextMenu
                    {
                        id: modifierCBmenu
                        Repeater
                        {
                            model: modifierModes
                            MenuItem { text: name; onClicked: settingsui.setSetting(key, code); }
                        }
                    }
                    Component.onCompleted:
                    {
                        var i
                        for (i=0 ; i < modifierModes.count ; i++)
                        {
                            var item = modifierModes.get(i)
                            if (settings[key] === item["code"])
                            {
                                currentIndex = i
                                return
                            }
                        }
                    }
                }
            }

            SectionHeader
            {
                //: Section header for Debug settings
                //% "Debug"
                text: qsTrId("debug-sect-header")
            }
            TextSwitch
            {
                //: Enable verbose mode to print more stuff on journal
                //% "Verbose mode"
                text: qsTrId("verbose-mode-sw")
                //: Verbose mode switch description
                //% "Print more information in Journal logs. Use 'devel-su journalctl -fa | grep toh' to see output."
                description: qsTrId("verbose-mode-desc")
                onCheckedChanged: if (checked !== settings["verboseMode"]) settingsui.setSetting("verboseMode", checked)
                width: parent.width - 2*Theme.paddingLarge
                Component.onCompleted: checked = settings["verboseMode"]
            }

            TextSwitch
            {
                id: nodeadkeysSwitch
                //: Switch to set 'nodeadkeys' in keymap variant
                //% "No deadkeys"
                text: qsTrId("nodeadkeys-sw")
                //: No deadkeys switch description
                //% "Set 'nodeadkeys' to keymap variant. Required for some keyboard layouts, e.g. fi, de."
                description: qsTrId("nodeadkeys-desc")
                width: parent.width - 2*Theme.paddingLarge
                automaticCheck: false
                onClicked:
                {
                    if (keymapVariant.value !== "nodeadkeys")
                        keymapVariant.value = "nodeadkeys"
                    else
                        keymapVariant.value = ""
                }
                Component.onCompleted: checked = (keymapVariant.value === "nodeadkeys")
            }
        }
    }

    ConfigurationValue
    {
        id: keymapVariant
        key: "/desktop/lipstick-jolla-home/variant"
        defaultValue: ""

        onValueChanged:
            nodeadkeysSwitch.checked = (value === "nodeadkeys")
    }
}

