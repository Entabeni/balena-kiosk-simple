var SerialPort = require('./serialport');
var cashPort = new SerialPort('/dev/ttyCashDrawer', { baudRate: 9600 });
cashPort.write('open');
