var SerialPortRelay = require('serialport');
var port = new SerialPortRelay('/dev/tty.usbserial-1440', { baudRate: 9600 });
// Turn Relay On
port.write(Buffer.from([0xA0, 0x01, 0x01, 0xA2]));
// Turn Relay Off
port.write(Buffer.from([0xA0, 0x01, 0x00, 0xA1]));
