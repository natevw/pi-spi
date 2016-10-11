/// <reference path="../node/node.d.ts" />

declare module 'pi-spi' {
  enum mode {
    CPHA = 0x01,
    CPOL = 0x02
  }

  enum order {
    MSB_FIRST = 0,
    LSB_FIRST = 1
  }

  /**
   * initialize an spi object
   * @param device will be usually be "/dev/spidev0.0" or "/dev/spidev0.0"
   */
  function initialize(device: string): spi;

  interface spi {
    /**
     * sets or gets (no args) the clock speed in Hz
     * default is 4e6 i.e. 4MHz
     * @param speed
     */
    clockSpeed(speed?: number): void|number;

    /**
     * sets or gets (no args) the "data mode".
     * default is no flags
     * @param mode
     */
    dataMode(mode?: mode): void|mode;

    /**
     * sets or gets (no args) the bit ordering
     * default is MSB_FIRST
     * @param order
     */
    bitOrder(order?: order): void|order;

    /**
     * transfer data for the longer of writeBuffer.length or readCount bytes.
     * @param writeBuffer
     * @param readCount
     * @param callback
     */
    transfer(writeBuffer: Buffer, callback: (error: any, data: Buffer) => void): void;
    transfer(writeBuffer: Buffer, readCount: number, callback: (error: any, data: Buffer) => void): void;

    /**
     * collects incount bytes whole writing as many \0 out.
     * @param readCount
     * @param callback
     */
    read(readCount: number, callback: (error: any, data: Buffer) => void): void;

    /**
     * writes buffer and ignoring response bytes
     * @param writeBuffer
     * @param callback
     */
    write(writeBuffer: Buffer, callback: (error: any, data: Buffer) => void): void;
  }
}