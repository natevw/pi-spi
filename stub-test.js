// this one is intended for testing stub binding on platforms where not actually implemented
var stubDevice = (process.platform === 'win32') ? "\\\\.\\NUL" : "/dev/null";
require("./").initialize(stubDevice).transfer(Buffer("-"), function (e,d) {
    console.log(e,d);
});