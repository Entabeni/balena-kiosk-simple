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
exports.mp = void 0;
var NodePrint_1 = __importDefault(require("./NodePrint"));
var escpos_1 = require("escpos");
var path = require("path");
var currency = "$";
var resortLogo = process.env.LOGO || "cooper";
var receiptVendorId = process.env.RECEIPT_VENDOR_ID || "0x0519";
var receiptProductId = process.env.RECEIPT_PROCDUCT_ID || "0x2013";
//@ts-ignore
function mp(relPath) {
    return path.resolve(__dirname, relPath);
}
exports.mp = mp;
var ReceiptPrinter = /** @class */ (function (_super) {
    __extends(ReceiptPrinter, _super);
    function ReceiptPrinter(mq, ws, qName, state) {
        var _this = _super.call(this, mq, ws, qName, state) || this;
        _this.mq = mq;
        _this.ws = ws;
        _this.qName = qName;
        _this.currentJobId = null;
        _this.numTimesCheckedPrinted = 0;
        _this.state = state;
        return _this;
    }
    ReceiptPrinter.prototype.getRounded = function (num) {
        //@ts-ignore
        return parseFloat(Math.round(num * 100) / 100).toFixed(2);
    };
    /** Function to ad space in reciept spool */
    ReceiptPrinter.prototype.addSpaces = function (str, strLength) {
        if (str.length < strLength) {
            var spaces = strLength - str.length;
            for (var i = 1; i <= spaces; i++) {
                str = str + " ";
            }
        }
        return str;
    };
    ReceiptPrinter.prototype.printReceiptHeader = function (printer, image, data) {
        printer
            .align("CT")
            .size(1, 1)
            .style("NORMAL");
        // Resort logo
        if (image) {
            //@ts-ignore
            printer.raster(image).feed(2);
        }
        // Resort address
        if (data && data.resortInfo && data.resortInfo.addressLine1) {
            printer.println(data.resortInfo.addressLine1);
        }
        if (data && data.resortInfo && data.resortInfo.addressLine2) {
            printer.println(data.resortInfo.addressLine2);
        }
        if (data && data.resortInfo && data.resortInfo.addressLine3) {
            printer.print(data.resortInfo.addressLine3);
        }
        if (data && data.resortInfo && data.resortInfo.zipCode) {
            printer.println(", " + data.resortInfo.zipCode);
        }
        else {
            printer.feed();
        }
        if (data && data.resortInfo && data.resortInfo.addressLine4) {
            printer.println(data.resortInfo.addressLine4);
        }
        printer.feed(2);
    };
    ReceiptPrinter.prototype.printReceiptCashoutBody = function (printer, data) {
        // Sales info
        var _this = this;
        printer
            .println("Sales Cashout")
            .feed(2)
            .align("LT");
        if (data.printTerminal) {
            printer.println("Cashout for POS terminal: " + data.printTerminal);
        }
        if (data.posSessionOpenedAtFormatted) {
            printer.println("POS Opened at: " + data.posSessionOpenedAtFormatted);
        }
        if (data.posSessionClosedAtFormatted) {
            printer.println("POS Closed at: " + data.posSessionClosedAtFormatted);
        }
        if (data.openingBalance) {
            printer.println("Opening Balance: " + currency + data.openingBalance);
        }
        if (data.closingBalance) {
            printer.println("Closing Balance: " + currency + data.closingBalance);
        }
        if (data.overShort) {
            printer.println("Over/Short:" + currency + data.overShort);
        }
        printer
            .feed()
            .println("------------------------------------------------")
            .feed();
        // Payment methods
        if (data.paymentMethods && data.paymentMethods.length) {
            printer
                .style("B")
                .println(this.addSpaces("Payment Method", 25) +
                this.addSpaces("Amount collected", 25))
                .style("NORMAL");
            data.paymentMethods.forEach(function (lineItem) {
                printer.println(_this.addSpaces("" + lineItem.paymentMethodName, 25) +
                    _this.addSpaces("" + currency + parseFloat(_this.getRounded(parseFloat(lineItem.amountCollected, 10))).toFixed(2), 25));
            });
        }
        // Separator;
        printer
            .feed()
            .println("------------------------------------------------")
            .feed();
        // Opening balances
        var grandTotal = 0;
        printer
            .style("B")
            .println("Opening Balances")
            .feed(1);
        if (data.openingBalances && data.openingBalances.length) {
            printer
                .style("B")
                .println(this.addSpaces("Name", 10) +
                this.addSpaces("Value", 10) +
                this.addSpaces("QTY", 10) +
                this.addSpaces("Total", 10))
                .style("NORMAL");
            data.openingBalances.forEach(function (lineItem) {
                grandTotal = grandTotal + parseFloat(lineItem.total);
                printer.println(_this.addSpaces("" + lineItem.denominationName, 10) +
                    _this.addSpaces("" + lineItem.denominationValue, 10) +
                    _this.addSpaces("" + lineItem.quantity, 10) +
                    _this.addSpaces("" + currency + _this.getRounded(parseFloat(lineItem.total)), 10));
            });
        }
        printer
            .style("B")
            .feed(1)
            .println(this.addSpaces("Grand Total", 30) +
            this.addSpaces("" + currency + this.getRounded(grandTotal), 25));
        // Separator;
        printer
            .feed()
            .println("------------------------------------------------")
            .feed();
        // Closing balances
        var grandTotalClosing = 0;
        printer
            .style("B")
            .println("Closing Balances")
            .feed(1);
        if (data.closingBalances && data.closingBalances.length) {
            printer
                .style("B")
                .println(this.addSpaces("Name", 10) +
                this.addSpaces("Value", 10) +
                this.addSpaces("QTY", 10) +
                this.addSpaces("Total", 10))
                .style("NORMAL");
            data.closingBalances.forEach(function (lineItem) {
                grandTotalClosing = grandTotalClosing + parseFloat(lineItem.total);
                printer.println(_this.addSpaces("" + lineItem.denominationName, 10) +
                    _this.addSpaces("" + lineItem.denominationValue, 10) +
                    _this.addSpaces("" + lineItem.quantity, 10) +
                    _this.addSpaces("" + currency + _this.getRounded(parseFloat(lineItem.total)), 10));
            });
        }
        printer
            .style("B")
            .feed(1)
            .println(this.addSpaces("Grand Total", 30) +
            this.addSpaces("" + currency + this.getRounded(grandTotalClosing), 25))
            .feed(2)
            .align("LT");
    };
    ReceiptPrinter.prototype.printReceiptBody = function (printer, data) {
        var _this = this;
        var _a;
        if (data && data.sale) {
            // Sales info
            printer
                .println("Sales Receipt")
                .println("Sale Number: " + data.sale.number)
                .println(data.sale.associateName ? data.sale.associateName : "")
                .println(data.sale.date)
                .feed(2)
                .align("LT");
            // Sales line items
            if (data.sale.lineItems && data.sale.lineItems.length) {
                printer
                    .style("B")
                    .println(this.addSpaces("Qty", 4) +
                    this.addSpaces("Description", 36) +
                    this.addSpaces("Amount", 7))
                    .style("NORMAL");
                data.sale.lineItems.forEach(function (lineItem) {
                    printer.println(_this.addSpaces("" + lineItem.quantity, 4) +
                        _this.addSpaces(lineItem.name, 36) +
                        _this.addSpaces("" + currency + parseFloat(lineItem.subTotal).toFixed(2), 7));
                    if (lineItem.guest) {
                        printer.println(_this.addSpaces("", 10) + _this.addSpaces(lineItem.guest, 25));
                    }
                    if (lineItem.discount && lineItem.discount.length) {
                        lineItem.discount.map(function (discount) {
                            printer.println(_this.addSpaces("", 10) +
                                _this.addSpaces(discount.name, 25) +
                                _this.addSpaces("(" + currency + parseFloat(discount.subTotal).toFixed(2) + ")", 7));
                        });
                    }
                });
            }
            // Separator;
            printer
                .feed()
                .println("------------------------------------------------")
                .feed();
            // Totals;
            printer.println(this.addSpaces("", 20) +
                this.addSpaces("Sub Total", 15) +
                this.addSpaces("" + currency + parseFloat((_a = data.sale.subTotal) !== null && _a !== void 0 ? _a : "0").toFixed(2), 7));
            Object.entries(data.sale.taxTotals).forEach(function (_a) {
                var key = _a[0], value = _a[1];
                printer.println(_this.addSpaces("", 20) +
                    _this.addSpaces(key, 15) +
                    //@ts-ignore
                    _this.addSpaces(value.toFixed(2), 7));
            });
            printer.println(this.addSpaces("", 20) +
                this.addSpaces("Total", 15) +
                this.addSpaces("" + currency + parseFloat(data.sale.total).toFixed(2), 7));
            // Separator;
            printer
                .feed()
                .println("------------------------------------------------")
                .feed();
            // Payments;
            if (data.sale.payments && data.sale.payments.length) {
                data.sale.payments.forEach(function (payment) {
                    printer
                        .println(_this.addSpaces("Payment Method", 20) +
                        _this.addSpaces(payment.name, 10))
                        .println(_this.addSpaces("Amount", 20) +
                        _this.addSpaces("" + currency + parseFloat(payment.amount).toFixed(2), 10));
                });
                printer.feed(2);
            }
        }
    };
    // printReceiptFooter(printer, data) {
    //   printer.println(data.footerText);
    // }
    ReceiptPrinter.prototype.beginPrinting = function (printJobData) {
        var _this = this;
        var message = JSON.parse(printJobData.message);
        var data = JSON.parse(message.data);
        if (receiptProductId) {
            try {
                var device_1 = new escpos_1.USB(receiptVendorId, receiptProductId);
                var options = { encoding: "GB18030" };
                var printer_1 = new escpos_1.Printer(device_1, options);
                var logo = mp("../../logos/logo-" + resortLogo + ".png");
                escpos_1.Image.load(logo, function (image) {
                    device_1.open(function () {
                        _this.printReceiptHeader(printer_1, image, data);
                        if (message.printJobType === "receipt") {
                            _this.printReceiptBody(printer_1, data);
                        }
                        else {
                            _this.printReceiptCashoutBody(printer_1, data);
                        }
                        printer_1
                            .feed(3)
                            .cut()
                            .close();
                        // Give it at least 5 seconds to print before finishing the job
                        setTimeout(function () {
                            _this.finishPrintJob(printJobData.id);
                            console.log("Reciept printed");
                        }, 300);
                    });
                });
            }
            catch (e) {
                setTimeout(function () {
                    _this.finishPrintJob(printJobData.id);
                    console.log("Reciept not printed");
                }, 300);
            }
        }
    };
    return ReceiptPrinter;
}(NodePrint_1.default));
exports.default = ReceiptPrinter;
