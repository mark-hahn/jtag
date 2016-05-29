
fs = require("fs");
tty = require("tty");

fd = fs.openSync('/dev/ttyACM0', 'r');
console.log("fd", fd);
var t = new tty.ReadStream(fd);
console.log(t.isTTY);

t.on('data', function(chunk) {
    console.log(chunk.toString());
});

t.on('end', function() {
    console.log("end");
});

// file = fs.readFileSync("test.data");
// console.log (file.toString());
