"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
//@ts-ignore
var serialport_1 = __importDefault(require("serialport"));
var CardRead = /** @class */ (function () {
    function CardRead(path) {
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
    CardRead.prototype.onPortOpen = function () {
        var _this = this;
        var buffer = Buffer.from([0x04, 0x00, 0x01, 0xdb, 0x4b]);
        this.intervalId = setInterval(function () {
            _this.port.write(buffer, function (err, result) {
                if (err) {
                }
                else {
                    _this.isConnected = true;
                }
            });
        }, 100);
        // Stop scanning after 30 seconds
        setTimeout(function () {
            if (_this.state === "scanning") {
                if (_this.intervalId) {
                    clearInterval(_this.intervalId);
                }
                _this.stopScanning();
            }
        }, this.timeout);
    };
    CardRead.prototype.onPortReceivedData = function (data) {
        if ((data.length === 20 || data.length === 14) && data[4] === 1) {
            //@ts-ignore
            this.setCardNumber(data.slice(6, 18).toString("hex"));
            if (this.intervalId) {
                clearInterval(this.intervalId);
            }
            this.stopScanning();
        }
    };
    CardRead.prototype.getConnected = function () {
        return this.isConnected;
    };
    CardRead.prototype.setCardNumber = function (cardNumber) {
        this.cardNumberHex = cardNumber;
    };
    CardRead.prototype.getCardNumber = function () {
        return this.cardNumberHex;
    };
    CardRead.prototype.readCardNumber = function (callback) {
        var _this = this;
        var that = this;
        if (this.state !== "scanning") {
            return;
        }
        setTimeout(function () {
            if (that.getCardNumber() == null) {
                _this.readCardNumber(callback);
            }
            else {
                callback(that.getCardNumber());
                that.setCardNumber(null);
            }
        }, 500);
    };
    CardRead.prototype.startScanning = function (timeout) {
        if (timeout === void 0) { timeout = 4500; }
        this.setCardNumber(null);
        this.timeout = timeout;
        this.port = new serialport_1.default("/dev/" + this.path, { baudRate: 57600 });
        this.port.on("open", this.onPortOpen);
        this.port.on("data", this.onPortReceivedData);
        this.state = "scanning";
    };
    CardRead.prototype.stopScanning = function () {
        this.port.close();
        this.state = "idle";
    };
    return CardRead;
}());
exports.default = CardRead;
