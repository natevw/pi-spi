/// <reference path="./pi-spi.d.ts" />

import * as pispi from 'pi-spi';

let spi = pispi.initialize('/dev/spidev0.0');
let test = new Buffer('Hello world');

spi.transfer(test, test.length, (error, data) => {
  if (error) {
    console.log(error);
    process.exit(-1);
  }

  let msg = `Got ${data.toString()} back.`;
  if (test.toString() === data.toString()) {
    console.log(msg);
  } else {
    // NOTE: this will likely happen unless MISO is jumpered to MOSI
    console.warn(msg);
    process.exit(-2);
  }
});