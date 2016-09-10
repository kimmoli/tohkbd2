harbour-ambience-tohkbd2
======

### Shortly 
* When TOHKBD base is attached to the phone for very first time
  * NFC ID is read by ```tohd``` and subbmitted to Jolla Store
  * Store install required packages automatically
  * During installation, EEPROM contents of TOHKBD are compared to certain vid/pid, and if they match, daemon is started aka `systemctl start harbour-tohkbd2`
* After this, everytime TOHKBD base is attached to the phone, EEPROM contents are checked by udev rule, which starts daemon
* When daemon is started it creates a uinput device which is used to send key-events to system
* When the keyboard part is connected to the base, interrupt is generated which triggers followig sequence:
  * power-up keyboard and check can we communicate over I2C with keyboard chip. If comms test fails, shutdown keyboard.
  * send SW_KEYPAD_SLIDE event (keypad opened), which is registered by mce to turn display on
  * Show a notification that keyboard is connected
  * Change virtual keyboard layout to minimal (done through tohkbd2-user daemon which writes/reads dconf)
  * Force screen orientation to landscape
* Keyboard presense is checked every 2 secs. If comms test fails:
  * shutdown keyboard
  * send SW_KEYPAD_SLIDE event (keypad closed)
  * show notification that keyboard is removed
  * change virtual keyboard layout back to what it was before connecting keyboard
* When key is pressed on keyboard:
  * Interrupt is generated
  * daemon reads input report from keyboard chip over I2C and processes it
  * If a valid key was pressed, this is sent by uinput device to the system
  * some of keys have special function, like starting a application. these are processed in the daemon.
  

### Installing through mer-obs repo:

mer-obs https://build.merproject.org/project/show/home:kimmoli:tohs

```
 devel-su
 ssu ar tohs http://repo.merproject.org/obs/home:/kimmoli:/tohs/sailfish_latest_armv7hl
 ssu ur
 pkcon repo-set-data tohs refresh-now true
 pkcon install harbour-ambience-tohkbd2
```

### References:

* TCA8424 datasheet: http://www.ti.com/lit/ds/symlink/tca8424.pdf
* TCA8424 evaluation module user's guide: www.ti.com/lit/ug/scdu004/scdu004.pdf

### Troubleshooting

* If your tohkbd does not work anymore (it doesn't say Keyboard connected anymore) and you notice that in fact the little pin of the back cover has broken off, one way to work around the problem (assuming you always have the tohkbd toh part attached) is to do `systemctl start harbour-tohkbd2 ; systemctl enable harbour-tohkbd2`. This has the same effect as if the pin was always present.
