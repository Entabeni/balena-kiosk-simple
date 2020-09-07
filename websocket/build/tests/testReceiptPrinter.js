var escpos = require("escpos");
var path = require("path");
var currency = "$";
function mp(relPath) {
    return path.resolve(__dirname, relPath);
}
var resortLogo = process.env.LOGO || "cooper";
var receiptVendorId = process.env.RECEIPT_VENDOR_ID || "0x0483";
var receiptProductId = process.env.RECEIPT_PROCDUCT_ID || "0x5743";
var ReceiptPrinter = /** @class */ (function () {
    function ReceiptPrinter() {
    }
    ReceiptPrinter.prototype.getRounded = function (num) {
        // @ts-ignore
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
            .align("ct")
            .size(1, 1)
            .style("normal");
        // Resort logo
        if (image) {
            printer.raster(image).feed(2);
        }
        printer
            .println("Sales Cashout")
            .println("Cashout for POS terminal: ")
            .println("POS Opened at: ")
            .println("POS Closed at: ")
            .println("Over/Short: $5.00")
            .align("lt");
        // Resort address
        if (data.resortInfo.addressLine1) {
        }
        if (data.resortInfo.addressLine2) {
            printer.println(data.resortInfo.addressLine2);
        }
        if (data.resortInfo.addressLine3) {
            printer.print(data.resortInfo.addressLine3);
        }
        if (data.resortInfo.zipCode) {
            printer.println(", " + data.resortInfo.zipCode);
        }
        else {
            printer.feed();
        }
        if (data.resortInfo.addressLine4) {
            printer.println(data.resortInfo.addressLine4);
        }
        printer.feed(2);
    };
    ReceiptPrinter.prototype.printReceiptCashoutBody = function (printer, data) {
        var _this = this;
        // Sales info
        printer
            .println("Sales Cashout")
            .println("Cashout for POS terminal: " + data.printTerminal)
            .println("POS Opened at: " + data.posSessionClosedAtFormatted)
            .println("POS Closed at: " + data.posSessionOpenedAtFormatted)
            .println("Opening Balance: " + currency + data.openingBalance)
            .println("Closing Balance: " + currency + data.closingBalance)
            .println("Over/Short: $5.00")
            .align("lt");
        printer
            .feed()
            .println("------------------------------------------")
            .feed();
        // Payment methods
        if (data.paymentMethods && data.paymentMethods.length) {
            printer
                .style("b")
                .println(this.addSpaces("Payment Method", 25) +
                this.addSpaces("Amount collected", 25))
                .style("normal");
            data.paymentMethods.forEach(function (lineItem) {
                printer.println(_this.addSpaces("" + lineItem.paymentMethodName, 25) +
                    _this.addSpaces("" + currency + parseFloat(_this.getRounded(lineItem.amountCollected)).toFixed(2), 25));
            });
        }
        // Separator;
        printer
            .feed()
            .println("------------------------------------------")
            .feed();
        // Opening balances
        var grandTotal = 0;
        printer
            .style("b")
            .println("Opening Balances")
            .feed(1);
        if (data.openingBalances && data.openingBalances.length) {
            printer
                .style("b")
                .println(this.addSpaces("Name", 10) +
                this.addSpaces("Value", 10) +
                this.addSpaces("QTY", 10) +
                this.addSpaces("Total", 10))
                .style("normal");
            data.openingBalances.forEach(function (lineItem) {
                grandTotal = grandTotal + parseInt(lineItem.total);
                printer.println(_this.addSpaces("" + lineItem.denominationName, 10) +
                    _this.addSpaces("" + lineItem.denominationValue, 10) +
                    _this.addSpaces("" + lineItem.quantity, 10) +
                    _this.addSpaces("" + currency + _this.getRounded(lineItem.total), 10));
            });
        }
        printer
            .style("b")
            .feed(1)
            .println(this.addSpaces("Grand Total", 30) +
            this.addSpaces("" + currency + this.getRounded(grandTotal), 25));
        // Separator;
        printer
            .feed()
            .println("------------------------------------------")
            .feed();
        // Closing balances
        var grandTotalClosing = 0;
        printer
            .style("b")
            .println("Closing Balances")
            .feed(1);
        if (data.closingBalances && data.closingBalances.length) {
            printer
                .style("b")
                .println(this.addSpaces("Name", 10) +
                this.addSpaces("Value", 10) +
                this.addSpaces("QTY", 10) +
                this.addSpaces("Total", 10))
                .style("normal");
            data.closingBalances.forEach(function (lineItem) {
                grandTotalClosing = grandTotalClosing + parseInt(lineItem.total);
                printer.println(_this.addSpaces("" + lineItem.denominationName, 10) +
                    _this.addSpaces("" + lineItem.denominationValue, 10) +
                    _this.addSpaces("" + lineItem.quantity, 10) +
                    _this.addSpaces("" + currency + _this.getRounded(lineItem.total), 10));
            });
        }
        printer
            .style("b")
            .feed(1)
            .println(this.addSpaces("Grand Total", 30) +
            this.addSpaces("" + currency + this.getRounded(grandTotalClosing), 25))
            .feed(2)
            .align("lt");
    };
    ReceiptPrinter.prototype.beginPrinting = function (printJobData) {
        var _this = this;
        var data = printJobData.data;
        try {
            var device_1 = new escpos.USB("0x0483", "0x5743");
            var options = { encoding: "GB18030" };
            var printer_1 = new escpos.Printer(device_1, options);
            var logo = mp("../../logos/logo-cooper.png");
            console.log("beginPrinting -> logo", logo);
            escpos.Image.load(logo, function (image) {
                console.log("beginPrinting -> image", image);
                device_1.open(function () {
                    _this.printReceiptHeader(printer_1, image, data);
                    printer_1
                        .feed(3)
                        .cut()
                        .close();
                    // Give it at least 5 seconds to print before finishing the job
                });
            });
        }
        catch (e) {
            setTimeout(function () { }, 3000);
        }
    };
    return ReceiptPrinter;
}());
var rp = new ReceiptPrinter();
setInterval(function () {
    rp.beginPrinting({
        data: {
            resortInfo: {
                addressLine1: "SkiCooper",
                addressLine2: "232 County Road 29",
                addressLine3: "Leadville, CO",
                addressLine4: "US",
            },
            sales: 1,
            printTerminal: "MyString",
            closingBalance: 40,
            openingBalance: 140,
            paymentMethods: [
                {
                    amountCollected: 5,
                    paymentMethodName: "Visa",
                },
                {
                    amountCollected: 4.99,
                    paymentMethodName: "Master Card",
                },
            ],
            closingBalances: [
                {
                    total: 20,
                    quantity: 2,
                    denominationName: "100's",
                    denominationValue: 10,
                },
                {
                    total: 20,
                    quantity: 1,
                    denominationName: "100's",
                    denominationValue: 20,
                },
            ],
            openingBalances: [
                {
                    total: 40,
                    quantity: 4,
                    denominationName: "100's",
                    denominationValue: 10,
                },
                {
                    total: 100,
                    quantity: 5,
                    denominationName: "100's",
                    denominationValue: 20,
                },
            ],
            posSessionClosedAtFormatted: "08:00 2019-11-11",
            posSessionOpenedAtFormatted: "15:00 2019-11-11",
        },
    });
}, 3500);
