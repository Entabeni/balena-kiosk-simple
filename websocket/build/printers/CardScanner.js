"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var NodePrint_1 = __importDefault(require("./NodePrint"));
var CardRead_1 = __importDefault(require("./CardRead"));
var CardScanner = /** @class */ (function (_super) {
    __extends(CardScanner, _super);
    function CardScanner(ws, qName) {
        var _this = _super.call(this, ws, qName) || this;
        // this.mq = mq;
        _this.ws = ws;
        _this.qName = qName;
        _this.currentJobId = null;
        _this.numTimesCheckedPrinted = 0;
        return _this;
    }
    /**
     * If no message ID, end the job
     * Else send update that the scan job is processing
     * Begin printing
     */
    CardScanner.prototype.print = function (printJobData) {
        this.currentJobId = printJobData.id;
        var message = JSON.parse(printJobData.message);
        if (!message.id) {
            // this.finishPrintJob(printJobData.id);
            return;
        }
        if (message.status === "cashDrawerOpen") {
            var cashPort_1 = new SerialPort("/dev/ttyCashDrawer", { baudRate: 9600 });
            cashPort_1.write("open", function () {
                cashPort_1.close();
            });
            // this.mq.deleteMessage(this.qName, printJobData.id, (success) =>
            //   console.log("deleted message", success)
            // );
            this.ws.updateScanJob(message.id, "cashDrawerOpenCompleted");
            this.currentJobId = null;
            // this.state.idle();
            return;
        }
        // Update print job status to processing
        // this.ws.updateScanJob(message.id, "processing");
        this.openScanner(printJobData);
    };
    CardScanner.prototype.openScanner = function (printJobData) {
        var _this = this;
        var cardReader = new CardRead_1.default("ttyDesktopReader");
        var successfulScan = false;
        cardReader.startScanning();
        cardReader.readCardNumber(function (cardNumberHex) {
            if (_this.currentState !== "idle") {
                var last8Digits = cardNumberHex.substr(cardNumberHex.length - 8, cardNumberHex.length);
                if (printJobData && printJobData.accessRecordId) {
                    _this.ws.updateAccessRecord(printJobData.accessRecordId, cardNumberHex);
                }
                successfulScan = true;
                // this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
                _this.ws.updateScanJob(printJobData.id, "completed", cardNumberHex);
                _this.currentJobId = null;
                // this.state.idle();
            }
        });
        setTimeout(function () {
            if (!successfulScan) {
                // this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
                _this.currentJobId = null;
                // this.state.idle();
            }
        }, 9000);
    };
    return CardScanner;
}(NodePrint_1.default));
exports.default = CardScanner;
