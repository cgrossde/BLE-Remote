# HM-10 BLE Remote Control for XBMC/KODI

![Prototypes](Prototype/Header.jpg?raw=true)

My infrared remote control annoyed me because it was unreliable and I always wanted to do something with those 7$ Bluetooth Low Energy modules (HM-10) I had lying around. So I prototyped this BLE remote control which communicates with a NodeJS endpoint. This endpoint then relays the commands to XBMC/Kodi using the JSON-RPC API. This is not a complete guide but more a help or inspiration for others tinkering with the HM-10.

## Parts

* Arduino Pro Micro (5V)
* HM-10 BLE module incl. breakout with logic level shifter (so you can safely use 5V on RX/TX)
* KY-040 rotary encoder
* 1x LED and 220 Ohm resistor
* 2x switches
* some 10k resistors and 1uF ceramic capacitors
* wire
* a 5V power source

## Prerequisites

Setup your BLE Module by connecting to it with a USB to serial and enter the following commands:

```
AT+IMME0
AT+TYPE1
AT+ROLE1
AT+MODE2
```

## Wiring

Check the pictures in the `Circuit` folder.

![Circuit](Circuit/BLE_VolControl.png?raw=true)

## NodeJS Endpoint

For the NodeJS endpoint to work you need Mac OS or Linux with a BLE module (integrated / USB / ...). Also enter XBMC/Kodi settings and make sure that under`Remote`, controling XBMC/Kodi from remote is allowed. In your `advancedSettings.xml` should be this:

```
<jsonrpc>
    <compactoutput>false</compactoutput>
    <tcpport>9090</tcpport>
</jsonrpc>
```

Install Bluez if you use linux!

Use `npm install` to install the node script dependecies!

Adapt `linux.js` or `mac.js` to your needs. Use `node discover.js` to find your BLE module and get it's UUID. The service and characteristic vars should not be changed.

**Note on linux:** I had some problems on linux. It did not reliably discover my BLE module. I don't no it's because of Bluez or my BLE USB dongle. That's why I later used my Mac to run the NodeJS endpoint. The `mac.js` script will automatically start and enable caffeine. It will also play a sound when the remote connected to it. After exiting the `mac.js` script caffeine will be instructed to terminat.

## Remote features

* Sleep mode after 30 seconds (4mA low power mode)
* On/Off switch (because 4mA is still to much)
* 2 modes: Volume control / Menu control

**Volume control mode:** Turn the rotary encoder to change volume. Push the rotary encoder button to play/pause the movie.

**Menu control:** Turn the rotary encoder to move up or down the movie/series list. Press to enter/select, press long to go back.

Check out the arduino source code to get an idea of what the status LED does.

## Check out the prototype folder for more images

![Prototyping](Prototype/IMG_2386.JPG?raw=true)

## License
    The MIT License (MIT)

    Copyright (c) 2014 Christoph Groß <gross@blubyte.de> (http://chris-labs.de)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.