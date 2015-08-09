# generate dbus interface and adaptor sources from xml templates
# tohkbd2, kimmoli 2015

TEMPLATE = aux

# user-daemon
system(qdbusxml2cpp ../user-daemon/config/com.kimmoli.tohkbd2user.xml -i ../user-daemon/src/userdaemon.h -a ../user-daemon/src/userAdaptor)
system(qdbusxml2cpp ../user-daemon/config/com.kimmoli.tohkbd2user.xml -p ../user-daemon/src/userInterface)
# daemom
system(qdbusxml2cpp ../daemon/config/com.kimmoli.tohkbd2.xml -i ../daemon/src/tohkeyboard.h -a ../daemon/src/daemonAdaptor)
system(qdbusxml2cpp ../daemon/config/com.kimmoli.tohkbd2.xml -p ../daemon/src/daemonInterface)
# settings-ui
system(qdbusxml2cpp ../settings-ui/config/com.kimmoli.tohkbd2settingsui.xml -i ../settings-ui/src/settingsui.h -a ../settings-ui/src/settingsuiAdaptor)
system(qdbusxml2cpp ../settings-ui/config/com.kimmoli.tohkbd2settingsui.xml -p ../settings-ui/src/settingsuiInterface)
