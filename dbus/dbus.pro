# tohkbd2 D-Bus related stuff
#
# kimmoli 2015

TEMPLATE = aux

# generate dbus interface and adaptor sources from xml templates

# user-daemon
system(qdbusxml2cpp com.kimmoli.tohkbd2user.xml -i ../user-daemon/src/userdaemon.h -a ../dbus/src/userdaemonAdaptor)
system(qdbusxml2cpp com.kimmoli.tohkbd2user.xml -p ../dbus/src/userdaemonInterface)

# daemom
system(qdbusxml2cpp com.kimmoli.tohkbd2.xml -i ../daemon/src/tohkeyboard.h -a ../dbus/src/daemonAdaptor)
system(qdbusxml2cpp com.kimmoli.tohkbd2.xml -p ../dbus/src/daemonInterface)

# settings-ui
system(qdbusxml2cpp com.kimmoli.tohkbd2settingsui.xml -i ../settings-ui/src/settingsui.h -a ../dbus/src/settingsuiAdaptor)
system(qdbusxml2cpp com.kimmoli.tohkbd2settingsui.xml -p ../dbus/src/settingsuiInterface)

# Install dbus related files

dbus_services.path = /usr/share/dbus-1/services/
dbus_services.files = com.kimmoli.tohkbd2settingsui.service \
                      com.kimmoli.tohkbd2user.service

interfaces.path = /usr/share/dbus-1/interfaces/
interfaces.files = com.kimmoli.tohkbd2settingsui.xml \
                   com.kimmoli.tohkbd2user.xml

systemd_services.path = /lib/systemd/system/
systemd_services.files = harbour-tohkbd2.service

busconfig.path = /etc/dbus-1/system.d/
busconfig.files = harbour-tohkbd2.conf

INSTALLS = dbus_services \
           interfaces \
           systemd_services \
           busconfig

OTHER_FILES += \
    com.kimmoli.tohkbd2.xml \
    com.kimmoli.tohkbd2settingsui.xml \
    com.kimmoli.tohkbd2user.xml \
    com.kimmoli.tohkbd2settingsui.service \
    com.kimmoli.tohkbd2user.service \
    harbour-tohkbd2.conf \
    harbour-tohkbd2.service
