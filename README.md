# pi-spi

Simple asyncronous node.js SPI library for Raspberry Pi (and likely other embedded Linux platforms that provide /dev/spidevN.N).


## Example

`npm install pi-pi`


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

Sets (or gets, if no argument provided) the clock speed in Hz. The Pi only supports [powers of 2 speeds](https://projects.drogon.net/understanding-spi-on-the-raspberry-pi/) and faster speeds might get derated a bit.

`TBD: constants for supported speeds`

### spi.dataMode([mode])

Sets (or gets, if no argument provided) the data mode which is *mumble mumble*.

`TBD: clarify and add OR-able flag constants`

### spi.bitOrder([order])

Sets (or gets, if no argument provided) the bit (not byte!) ordering. (I think any non-zero integer will flip it to least-significant-bit first but don't quote me on that.)

`TBD: clarify and add constants`

### spi.transfer(outbuffer, incount, cb)

Transfers data for the longer of `outbuffer.length` or `incount` bytes. If successfully, the second parameter to your callback will be a buffer of length `incount`.

### spi.read(incount, cb)

Collects incount bytes while writing as many `\0` out.

### spi.write(outbuffer, cb)

Writes outbuffer, ignoring response bytes.

Note that if there was an error opening the device, the `transfer`/`read`/`write` calls will fail each time called. I may [revise the initialize method](https://github.com/natevw/pi-spi/issues/2#issuecomment-27588982) so to allow you to handle the error better.



## Compatibility Note

This library is subject to change as it is intended for compatibility with `$somethingElse`.

`TBD: more details when public`


## License

© 2013 Nathan Vander Wilt.

`TBD: insert BSD-2-Clause text here`