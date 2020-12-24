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
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
// @ts-ignore
var uuid_1 = require("uuid");
var fs_1 = __importDefault(require("fs"));
var https_1 = __importDefault(require("https"));
var NodePrint_1 = __importStar(require("./NodePrint"));
var node_native_printer_1 = __importDefault(require("node-native-printer"));
node_native_printer_1.default.setPrinter("Magicard_600");
var pathCardPrint = require("path");
var CardPrinter = /** @class */ (function (_super) {
    __extends(CardPrinter, _super);
    function CardPrinter(mq, ws, qName, state) {
        var _this = _super.call(this, mq, ws, qName, state) || this;
        _this.mq = mq;
        _this.ws = ws;
        _this.qName = qName;
        _this.finishPrintJob = _super.prototype.finishPrintJob;
        _this.currentJobId = null;
        _this.numTimesCheckedPrinted = 0;
        _this.state = state;
        return _this;
    }
    CardPrinter.prototype.downloadPDF = function (url, callback) {
        var directory = NodePrint_1.mp("../downloads/");
        fs_1.default.readdir(directory, function (err, files) {
            if (err)
                throw err;
            for (var _i = 0, files_1 = files; _i < files_1.length; _i++) {
                var file_1 = files_1[_i];
                fs_1.default.unlink(pathCardPrint.join(directory, file_1), function (err) {
                    if (err)
                        throw err;
                });
            }
        });
        var filename = uuid_1.v4() + ".pdf";
        var file = fs_1.default.createWriteStream(NodePrint_1.mp("../downloads/" + filename));
        https_1.default.get(url, function (response) {
            response.pipe(file);
            file.on("finish", function () {
                file.close();
                callback(filename);
            });
        });
    };
    CardPrinter.prototype.printingStarted = function (printJobData, jobId, message) {
        this.mq.deleteMessage(this.qName, printJobData.id, function (success) { });
        this.currentJobId = null;
        this.state.idle();
    };
    /**
     * Listen for when the printing starts
     * Loop every 500ms and when it starts, run printing started to scan card
     */
    CardPrinter.prototype.checkPrintingStarted = function (printJobData, printString, message) {
        var _this = this;
        var jobId = printString &&
            printString.match(/-([0-9]+)/) &&
            printString.match(/-([0-9]+)/)[1];
        var printerInfo = node_native_printer_1.default.printerInfo("Magicard_600");
        var job = printerInfo.jobs.find(function (job) { return job.id === jobId; });
        this.numTimesCheckedPrinted++;
        if (((job && (job.status === "processing" || job.status === "pending")) ||
            !job) &&
            this.numTimesCheckedPrinted === 27) {
            this.numTimesCheckedPrinted = 0;
            this.finishPrintJob(printJobData.id);
            return;
        }
        if ((job && job.status === "processing") ||
            printerInfo.CUPSOptions["printer-state"] === "3") {
            this.numTimesCheckedPrinted = 0;
            setTimeout(function () {
                _this.printingStarted(printJobData, jobId, message);
            }, 200);
        }
        else {
            setTimeout(function () { return _this.checkPrintingStarted(printJobData, printString, message); }, 500);
        }
    };
    /**
     * Use PDF in payload,
     * If no PDF, end the print job
     * Download PDF and print it in FITPLOT mode
     * and listen for when it starts
     */
    CardPrinter.prototype.beginPrinting = function (printJobData) {
        var _this = this;
        var message = JSON.parse(printJobData.message);
        // Update print job status to processing
        var pdfUrl = JSON.parse(message.data).pdfUrl;
        if (!pdfUrl) {
            this.finishPrintJob(printJobData.id);
            return;
        }
        this.downloadPDF(pdfUrl, function (filename) {
            setTimeout(function () {
                var options = { fitplot: true };
                var printString = node_native_printer_1.default.print(NodePrint_1.mp("../downloads/" + filename), options, "Magicard_600");
                _this.checkPrintingStarted(printJobData, printString, message);
            }, 1500);
        });
    };
    return CardPrinter;
}(NodePrint_1.default));
exports.default = CardPrinter;
