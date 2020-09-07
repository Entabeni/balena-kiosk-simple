//@ts-ignore
import SerialPort from "serialport";

class CardRead {
  path: string;
  intervalId: null | NodeJS.Timeout;
  cardNumberHex: null | string;
  isConnected: boolean;
  port: SerialPort;
  timeout: number;
  state: string;
  constructor(path: string) {
    this.path = path;
    this.intervalId = null;
    this.cardNumberHex = null;
    this.onPortOpen = this.onPortOpen.bind(this);
    this.isConnected = true;
    this.timeout = 4500;
    this.getConnected = this.getConnected.bind(this);
    this.onPortReceivedData = this.onPortReceivedData.bind(this);

    this.state = "idle";
  }

  onPortOpen() {
    const buffer = Buffer.from([0x04, 0x00, 0x01, 0xdb, 0x4b]);
    this.intervalId = setInterval(() => {
      this.port.write(buffer, (err, result) => {
        if (err) {
        } else {
          this.isConnected = true;
        }
      });
    }, 100);

    // Stop scanning after 30 seconds
    setTimeout(() => {
      if (this.state === "scanning") {
        if (this.intervalId) {
          clearInterval(this.intervalId);
        }
        this.stopScanning();
      }
    }, this.timeout);
  }

  onPortReceivedData(data: number[]) {
    if ((data.length === 20 || data.length === 14) && data[4] === 1) {
      //@ts-ignore
      this.setCardNumber(data.slice(6, 18).toString("hex"));
      if (this.intervalId) {
        clearInterval(this.intervalId);
      }
      this.stopScanning();
    }
  }

  getConnected() {
    return this.isConnected;
  }
  setCardNumber(cardNumber: string | null) {
    this.cardNumberHex = cardNumber;
  }

  getCardNumber() {
    return this.cardNumberHex;
  }

  readCardNumber(callback: (cb: string | null) => void) {
    const that = this;

    if (this.state !== "scanning") {
      return;
    }
    setTimeout(() => {
      if (that.getCardNumber() == null) {
        this.readCardNumber(callback);
      } else {
        callback(that.getCardNumber());
        that.setCardNumber(null);
      }
    }, 500);
  }

  startScanning(timeout = 4500) {
    this.setCardNumber(null);
    this.timeout = timeout;
    this.port = new SerialPort(`/dev/${this.path}`, { baudRate: 57600 });
    this.port.on("open", this.onPortOpen);
    this.port.on("data", this.onPortReceivedData);
    this.state = "scanning";
  }

  stopScanning() {
    this.port.close();
    this.state = "idle";
  }
}

export default CardRead;
