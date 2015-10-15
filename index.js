var fs = require('fs'),
    _spi = require("./build/Release/spi_binding");

exports.mode = {
    CPHA: 0x01,
    CPOL: 0x02
};

exports.order = {
    MSB_FIRST: 0,
    LSB_FIRST: 1
};

exports.initialize = function (dev) {
    var spi = {},
        _fd = fs.openSync(dev, 'r+'),
        _speed = 4e6,
        _mode = 0,
        _order = exports.order.MSB_FIRST;
    
    spi.clockSpeed = function (speed) {
        if (arguments.length < 1) return _speed;
        else if (typeof speed === 'number') _speed = speed;
        else throw TypeError("Clock speed must be a number.");
    };
    spi.dataMode = function (mode) {
        if (arguments.length < 1) return _mode;
        else if (typeof mode === 'number') _mode = mode;
        else throw TypeError("Data mode should be CPHA or CPOL.");
    };
    spi.bitOrder = function (order) {
        if (arguments.length < 1) return _order;
        else if (typeof order === 'number') _order = order;
        else throw TypeError("Bit order should be MSB_FIRST or LSB_FIRST.");
    };
    
    
    function _transfer(w,r,cb) {
        _spi.Transfer(_fd, _speed, _mode, _order, w, r, cb);
    }
    
    spi.write = function (writebuf, cb) {
        if (!Buffer.isBuffer(writebuf)) throw TypeError("Write data is not a buffer");
        if (typeof cb !== 'function') throw TypeError("Callback not provided");
        _transfer(writebuf, 0, cb);
    };
    spi.read = function (readcount, cb) {
        if (typeof readcount !== 'number') throw TypeError("Read count is not a number");
        if (typeof cb !== 'function') throw TypeError("Callback not provided");
        _transfer(null, readcount, cb);
    };
    spi.transfer = function (writebuf, readcount, cb) {
        if (!Buffer.isBuffer(writebuf)) throw TypeError("Write data is not a buffer");
        if (typeof readcount === 'function') {
            cb = readcount;
            readcount = writebuf.length;
        } else if (typeof readcount !== 'number') throw TypeError("Read count is not a number");
        if (typeof cb !== 'function') throw TypeError("Callback not provided");
        _transfer(writebuf, readcount, cb);
    };
    spi.close = function () {
        fs.close(_fd);
    };
    
    return spi;
};


