# pi-spi

Simple asyncronous node.js SPI library for Raspberry Pi (and likely other embedded Linux platforms that provide /dev/spidevN.N).


## Example

`npm install pi-spi`


```
var SPI = require('pi-spi');

var spi = SPI.initialize("/dev/spidev0.0"),
    test = Buffer("Hello, World!");

// reads and writes simultaneously
spi.transfer(test, test.length, function (e,d) {
    if (e) console.error(e);
    else console.log("Got \""+d.toString()+"\" back.");
    
    if (test.toString() === d.toString()) {
        console.log(msg);
    } else {
        // NOTE: this will likely happen unless MISO is jumpered to MOSI
        console.warn(msg);
        process.exit(-2);
    }
});
```

Probably requires running node under `sudo` for SPI permissions, unless you've used [Wiring Pi's gpio utility](https://projects.drogon.net/raspberry-pi/wiringpi/the-gpio-utility/) or otherwise adjusted device permissions.

## API

### spi = SPI.initialize(device)

`device` will usually be "/dev/spidev0.0" or "/dev/spidev0.0". You will first need to enable the `spi-bcm2708` kernel module [e.g. these instructions](http://scruss.com/blog/2013/01/19/the-quite-rubbish-clock/#spi) or similar for your platform. As mentioned above, by default this device requires root permissions and so you'll either need to change this or run your script with according privilege.

### spi.clockSpeed([speed])

Sets (or gets, if no argument provided) the clock speed in Hz. Defaults to `4e6`, i.e. 4MHz. The Pi only supports [powers of 2 speeds](https://projects.drogon.net/understanding-spi-on-the-raspberry-pi/) and faster speeds might get derated a bit.

### spi.dataMode([mode])

Sets (or gets, if no argument provided) the "data mode" (clock phase and polarity) to e.g. `SPI.mode.CPHA | SPI.mode.CPOL`. Default is no flags.

### spi.bitOrder([order])

Sets (or gets, if no argument provided) the bit ordering. Default is `SPI.order.MSB_FIRST` or you can set `SPI.order.LSB_FIRST`.

Note that this is **bit** ordering, not *bytes* — byte ordering is up to your application.

### spi.transfer(outbuffer, [incount,] cb)

Transfers data for the longer of `outbuffer.length` or `incount` bytes. If successfully, the second parameter to your callback will be a buffer of length `incount` (which defaults to `outbuffer.length` if not provided).

### spi.read(incount, cb)

Collects incount bytes while writing as many `\0` out.

### spi.write(outbuffer, cb)

Writes outbuffer, ignoring response bytes.

Note that if there was an error opening the device, the `transfer`/`read`/`write` calls will fail each time called. I may [revise the initialize method](https://github.com/natevw/pi-spi/issues/2#issuecomment-27588982) so to allow you to handle the error better.


## License

Copyright © 2013, Nathan Vander Wilt.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
