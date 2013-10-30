exports.initialize = function (dev) {
    
    var spi = {},
        _dirty = true,
        _speed = 4e6,
        _mode = null,       // does RasPi support?
        _order = 0;
    
    spi.clockSpeed = function (speed) {
        if (arguments.length < 1) return _speed;
        else if (speed !== _speed) {
            _speed = speed;
            _dirty = true;
        }
    };
    spi.dataMode = function (mode) {
        if (arguments.length < 1) return _mode;
        else if (mode !== _mode) {
            _mode = mode;
            _dirty = true;
        }
    };
    spi.bitOrder = function (order) {
        if (arguments.length < 1) return _order;
        else if (order !== _order) {
            _order = order;
            _dirty = true;
        }
    };
    
    function _configureIfDirty() {
        if (!_dirty) return;
        // TODO: the dirty work
    }
    
    var TODO = Error("Not implemented :-(");
    
    spi.write = function (writebuf, cb) {
        setTimeout(cb.bind(null, TODO),0);
    };
    spi.read = function (readcount, cb) {
        setTimeout(cb.bind(null, TODO),0);
    };
    spi.transfer = function (writebuf, readcount, cb) {
        setTimeout(cb.bind(null, TODO),0);
    };
    
    return spi;
}