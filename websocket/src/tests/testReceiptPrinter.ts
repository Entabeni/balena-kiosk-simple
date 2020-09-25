const escpos = require("escpos");
const path = require("path");
const currency = "$";
function mp(relPath) {
  return path.resolve(__dirname, relPath);
}
const resortLogo = process.env.LOGO || "cooper";
const receiptVendorId = process.env.RECEIPT_VENDOR_ID || "0x0519";
const receiptProductId = process.env.RECEIPT_PROCDUCT_ID || "0x2013";
class ReceiptPrinter {
  getRounded(num: number) {
    // @ts-ignore
    return parseFloat(Math.round(num * 100) / 100).toFixed(2);
  }
  /** Function to ad space in reciept spool */
  addSpaces(str, strLength) {
    if (str.length < strLength) {
      const spaces = strLength - str.length;
      for (var i = 1; i <= spaces; i++) {
        str = str + " ";
      }
    }
    return str;
  }

  printReceiptHeader(printer, image, data) {
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
    } else {
      printer.feed();
    }

    if (data.resortInfo.addressLine4) {
      printer.println(data.resortInfo.addressLine4);
    }

    printer.feed(2);
  }

  printReceiptCashoutBody(printer, data) {
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
        .println(
          this.addSpaces("Payment Method", 25) +
            this.addSpaces("Amount collected", 25)
        )
        .style("normal");

      data.paymentMethods.forEach((lineItem) => {
        printer.println(
          this.addSpaces(`${lineItem.paymentMethodName}`, 25) +
            this.addSpaces(
              `${currency}${parseFloat(
                this.getRounded(lineItem.amountCollected)
              ).toFixed(2)}`,
              25
            )
        );
      });
    }

    // Separator;
    printer
      .feed()
      .println("------------------------------------------")
      .feed();
    // Opening balances

    let grandTotal = 0;
    printer
      .style("b")
      .println("Opening Balances")
      .feed(1);

    if (data.openingBalances && data.openingBalances.length) {
      printer
        .style("b")
        .println(
          this.addSpaces("Name", 10) +
            this.addSpaces("Value", 10) +
            this.addSpaces("QTY", 10) +
            this.addSpaces("Total", 10)
        )
        .style("normal");

      data.openingBalances.forEach((lineItem) => {
        grandTotal = grandTotal + parseInt(lineItem.total);
        printer.println(
          this.addSpaces(`${lineItem.denominationName}`, 10) +
            this.addSpaces(`${lineItem.denominationValue}`, 10) +
            this.addSpaces(`${lineItem.quantity}`, 10) +
            this.addSpaces(`${currency}${this.getRounded(lineItem.total)}`, 10)
        );
      });
    }

    printer
      .style("b")
      .feed(1)
      .println(
        this.addSpaces("Grand Total", 30) +
          this.addSpaces(`${currency}${this.getRounded(grandTotal)}`, 25)
      );

    // Separator;
    printer
      .feed()
      .println("------------------------------------------")
      .feed();
    // Closing balances
    let grandTotalClosing = 0;
    printer
      .style("b")
      .println("Closing Balances")
      .feed(1);

    if (data.closingBalances && data.closingBalances.length) {
      printer
        .style("b")
        .println(
          this.addSpaces("Name", 10) +
            this.addSpaces("Value", 10) +
            this.addSpaces("QTY", 10) +
            this.addSpaces("Total", 10)
        )
        .style("normal");

      data.closingBalances.forEach((lineItem) => {
        grandTotalClosing = grandTotalClosing + parseInt(lineItem.total);
        printer.println(
          this.addSpaces(`${lineItem.denominationName}`, 10) +
            this.addSpaces(`${lineItem.denominationValue}`, 10) +
            this.addSpaces(`${lineItem.quantity}`, 10) +
            this.addSpaces(`${currency}${this.getRounded(lineItem.total)}`, 10)
        );
      });
    }

    printer
      .style("b")
      .feed(1)
      .println(
        this.addSpaces("Grand Total", 30) +
          this.addSpaces(`${currency}${this.getRounded(grandTotalClosing)}`, 25)
      )
      .feed(2)
      .align("lt");
  }

  beginPrinting(printJobData) {
    const data = printJobData.data;
    try {
      const device = new escpos.USB("0x0483", "0x5743");
      const options = { encoding: "GB18030" };
      const printer = new escpos.Printer(device, options);
      const logo = mp(`../../logos/logo-cooper.png`);
      console.log("beginPrinting -> logo", logo);

      escpos.Image.load(logo, (image) => {
        console.log("beginPrinting -> image", image);
        device.open(() => {
          this.printReceiptHeader(printer, image, data);
          printer
            .feed(3)
            .cut()
            .close();

          // Give it at least 5 seconds to print before finishing the job
        });
      });
    } catch (e) {
      setTimeout(() => {}, 3000);
    }
  }
}

const rp = new ReceiptPrinter();
setInterval(() => {
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
