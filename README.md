tohkbd2-user daemon
=================

userspace daemon for tohkbd2

to test translations, first stop current daemon (as nemo)

```dbus-send --type=method_call --dest=com.kimmoli.tohkbd2user / com.kimmoli.tohkbd2user.quit```

then start it with language you want

```LANG=es harbour-tohkbd2-user```

Translations are handled in transifex https://www.transifex.com/projects/p/tohkbd2-user/
