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
var fs_1 = __importDefault(require("fs"));
var node_native_printer_1 = __importDefault(require("node-native-printer"));
var printerName = process.env.LABEL_PRINTER || "DYMO_LabelWriter_450";
node_native_printer_1.default.setPrinter(printerName);
var pathCard = require("path");
var printerSide = process.env.PRINTER_SIDE || "Left";
function splitIntoLines(input, len) {
    var i;
    var output = [];
    var lineSoFar = "";
    var temp;
    var words = input.split(" ");
    for (i = 0; i < words.length;) {
        // check if adding this word would exceed the len
        temp = addWordOntoLine(lineSoFar, words[i]);
        if (temp.length > len) {
            if (lineSoFar.length == 0) {
                lineSoFar = temp; // force to put at least one word in each line
                i++; // skip past this word now
            }
            output.push(lineSoFar.replace(/,\s*$/, "").trim()); // put line into output
            lineSoFar = ""; // init back to empty
        }
        else {
            lineSoFar = temp; // take the new word
            i++; // skip past this word now
        }
    }
    if (lineSoFar.length > 0) {
        output.push(lineSoFar);
    }
    return output;
}
function addWordOntoLine(line, word) {
    if (line.length != 0) {
        line += " ";
    }
    return (line += word);
}
var LabelPrinter = /** @class */ (function (_super) {
    __extends(LabelPrinter, _super);
    function LabelPrinter(mq, ws, qName, state) {
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
    LabelPrinter.prototype.beginPrinting = function (printJobData) {
        var _this = this;
        var message = JSON.parse(printJobData.message);
        var addressJson = JSON.parse(JSON.parse(message.data));
        var purchaserName = addressJson.purchaserName, address1 = addressJson.address1, address2 = addressJson.address2, state = addressJson.state, zip = addressJson.zip, country = addressJson.country, city = addressJson.city;
        var addressLine = address2 ? address1 + ", " + address2 : address1;
        var addressLineSplit = splitIntoLines(addressLine, 27);
        var cityStateZip = splitIntoLines(city + ", " + state + ", " + zip, 27);
        var cityStateZipBetween = cityStateZip.length > 1 ? ", " : ",\n";
        function mpTest(relPath) {
            return pathCard.resolve(__dirname, relPath);
        }
        fs_1.default.unlink(mpTest("./log.txt"), function () {
            var finalString = purchaserName +
                ",\n" +
                addressLineSplit.join(",\n") +
                ",\n" +
                cityStateZip.join(",\n") +
                cityStateZipBetween +
                country;
            fs_1.default.appendFile(mpTest("./log.txt"), finalString, function (err) {
                if (err) {
                    _this.finishPrintJob(printJobData.id);
                }
                else {
                    node_native_printer_1.default.print(mpTest("./log.txt"), {
                        landscape: true,
                        InputSlot: printerSide,
                        pageSize: "24_mm__1___Label__Auto_",
                    });
                    _this.finishPrintJob(printJobData.id);
                }
            });
        });
    };
    return LabelPrinter;
}(NodePrint_1.default));
exports.default = LabelPrinter;
