const SerialPort = require('./serialport');

const cashPort = new SerialPort('/dev/ttyCashDrawer', { baudRate: 9600 });

cashPort.write('open');
