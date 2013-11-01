var SPI = require("./index");

var spi = SPI.initialize("/dev/spidev0.0"),
    test = Buffer("Hello, World!");
spi.transfer(test, test.length, function (e,d) {
    if (e) {
        console.error(e);
        process.exit(-1);
    }
    
    var msg = "Got \""+d.toString()+"\" back.";
    if (test.toString() === d.toString()) {
        console.log(msg);
    } else {
        // NOTE: this will likely happen unless MISO is jumpered to MOSI
        console.warn(msg);
        process.exit(-2);
    }
});