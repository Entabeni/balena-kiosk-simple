const ModbusRTUTest = require('modbus-serial')

const clientTest = new ModbusRTUTest()

clientTest.connectRTU('/dev/tty.usbserial-AK068AGR', { baudRate: 9600 }, read)

function read() {
  clientTest.setID(1)
  setInterval(() => {
    clientTest.readInputRegisters(0, 1).then(console.log)
  }, 10000)
}
