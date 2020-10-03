import NodePrinter from "./NodePrint";

import { USB, Printer, Image } from "escpos";
const path = require("path");

const currency: string = "$";
const resortLogo = process.env.LOGO || "cooper";
const receiptVendorId = process.env.RECEIPT_VENDOR_ID || "0x0519";
const receiptProductId = process.env.RECEIPT_PROCDUCT_ID || "0x2013";
//@ts-ignore
export function mp(relPath: string) {
  return path.resolve(__dirname, relPath);
}
interface ReciptData {
  resortInfo?: {
    addressLine1: string;
    addressLine2: string;
    addressLine3: string;
    zipCode: string;
    addressLine4: string;
  };
  printTerminal?: string;
  posSessionOpenedAtFormatted?: string;
  posSessionClosedAtFormatted?: string;
  s;
  openingBalance?: string;
  closingBalance?: string;
  overShort?: string;
  paymentMethods?: {
    paymentMethodName: string;
    amountCollected: string;
  }[];
  openingBalances: {
    total: string;
    denominationName: string;
    denominationValue: string;
    quantity: string;
  }[];
  closingBalances: {
    total: string;
    denominationName: string;
    denominationValue: string;
    quantity: string;
  }[];
  sale?: {
    associateName: string;
    number: string;
    date: string;
    subTotal: string;
    lineItems: {
      quantity: string;
      name: string;
      subTotal: string;
      guest: string;
      discount: { name: string; subTotal: string }[];
    }[];
    taxTotals: string;
    total: string;
    payments: { name: string; amount: string }[];
  };
}
class ReceiptPrinter extends NodePrinter {
  mq: any;
  ws: any;
  qName: any;
  currentJobId: any;
  intervalId: any;
  currentState: any;
  numTimesCheckedPrinted: any;
  state: any;
  constructor(mq: any, ws: any, qName: any, state: any) {
    super(mq, ws, qName, state);
    this.mq = mq;
    this.ws = ws;
    this.qName = qName;

    this.currentJobId = null;
    this.numTimesCheckedPrinted = 0;
    this.state = state;
  }
  getRounded(num: number) {
    //@ts-ignore
    return parseFloat(Math.round(num * 100) / 100).toFixed(2);
  }

  /** Function to ad space in reciept spool */
  addSpaces(str: string, strLength: number) {
    if (str.length < strLength) {
      const spaces = strLength - str.length;
      for (var i = 1; i <= spaces; i++) {
        str = str + " ";
      }
    }
    return str;
  }

  printReceiptHeader(printer: Printer, image: Image | Error, data: ReciptData) {
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
    } else {
      printer.feed();
    }

    if (data && data.resortInfo && data.resortInfo.addressLine4) {
      printer.println(data.resortInfo.addressLine4);
    }

    printer.feed(2);
  }
  printReceiptCashoutBody(printer: Printer, data: ReciptData) {
    // Sales info

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
        .println(
          this.addSpaces("Payment Method", 25) +
            this.addSpaces("Amount collected", 25)
        )
        .style("NORMAL");

      data.paymentMethods.forEach((lineItem) => {
        printer.println(
          this.addSpaces(`${lineItem.paymentMethodName}`, 25) +
            this.addSpaces(
              `${currency}${parseFloat(
                this.getRounded(parseInt(lineItem.amountCollected, 10))
              ).toFixed(2)}`,
              25
            )
        );
      });
    }

    // Separator;
    printer
      .feed()
      .println("------------------------------------------------")
      .feed();
    // Opening balances

    let grandTotal = 0;
    printer
      .style("B")
      .println("Opening Balances")
      .feed(1);

    if (data.openingBalances && data.openingBalances.length) {
      printer
        .style("B")
        .println(
          this.addSpaces("Name", 10) +
            this.addSpaces("Value", 10) +
            this.addSpaces("QTY", 10) +
            this.addSpaces("Total", 10)
        )
        .style("NORMAL");

      data.openingBalances.forEach((lineItem) => {
        grandTotal = grandTotal + parseInt(lineItem.total);
        printer.println(
          this.addSpaces(`${lineItem.denominationName}`, 10) +
            this.addSpaces(`${lineItem.denominationValue}`, 10) +
            this.addSpaces(`${lineItem.quantity}`, 10) +
            this.addSpaces(
              `${currency}${this.getRounded(parseInt(lineItem.total, 10))}`,
              10
            )
        );
      });
    }

    printer
      .style("B")
      .feed(1)
      .println(
        this.addSpaces("Grand Total", 30) +
          this.addSpaces(`${currency}${this.getRounded(grandTotal)}`, 25)
      );

    // Separator;
    printer
      .feed()
      .println("------------------------------------------------")
      .feed();
    // Closing balances
    let grandTotalClosing = 0;
    printer
      .style("B")
      .println("Closing Balances")
      .feed(1);

    if (data.closingBalances && data.closingBalances.length) {
      printer
        .style("B")
        .println(
          this.addSpaces("Name", 10) +
            this.addSpaces("Value", 10) +
            this.addSpaces("QTY", 10) +
            this.addSpaces("Total", 10)
        )
        .style("NORMAL");

      data.closingBalances.forEach((lineItem) => {
        grandTotalClosing = grandTotalClosing + parseInt(lineItem.total);
        printer.println(
          this.addSpaces(`${lineItem.denominationName}`, 10) +
            this.addSpaces(`${lineItem.denominationValue}`, 10) +
            this.addSpaces(`${lineItem.quantity}`, 10) +
            this.addSpaces(
              `${currency}${this.getRounded(parseInt(lineItem.total, 10))}`,
              10
            )
        );
      });
    }

    printer
      .style("B")
      .feed(1)
      .println(
        this.addSpaces("Grand Total", 30) +
          this.addSpaces(`${currency}${this.getRounded(grandTotalClosing)}`, 25)
      )
      .feed(2)
      .align("LT");
  }
  printReceiptBody(printer: Printer, data: ReciptData) {
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
          .println(
            this.addSpaces("Qty", 4) +
              this.addSpaces("Description", 36) +
              this.addSpaces("Amount", 7)
          )
          .style("NORMAL");

        data.sale.lineItems.forEach((lineItem) => {
          printer.println(
            this.addSpaces(`${lineItem.quantity}`, 4) +
              this.addSpaces(lineItem.name, 36) +
              this.addSpaces(
                `${currency}${parseInt(lineItem.subTotal).toFixed(2)}`,
                7
              )
          );
          if (lineItem.guest) {
            printer.println(
              this.addSpaces("", 10) + this.addSpaces(lineItem.guest, 25)
            );
          }
          if (lineItem.discount && lineItem.discount.length) {
            lineItem.discount.map((discount) => {
              printer.println(
                this.addSpaces("", 10) +
                  this.addSpaces(discount.name, 25) +
                  this.addSpaces(
                    `(${currency}${parseInt(discount.subTotal).toFixed(2)})`,
                    7
                  )
              );
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
      printer.println(
        this.addSpaces("", 20) +
          this.addSpaces("Sub Total", 15) +
          this.addSpaces(
            `${currency}${parseInt(data.sale.subTotal ?? "0").toFixed(2)}`,
            7
          )
      );
      Object.entries(data.sale.taxTotals).forEach(([key, value]) => {
        printer.println(
          this.addSpaces("", 20) +
            this.addSpaces(key, 15) +
            //@ts-ignore
            this.addSpaces(value.toFixed(2), 7)
        );
      });
      printer.println(
        this.addSpaces("", 20) +
          this.addSpaces("Total", 15) +
          this.addSpaces(
            `${currency}${parseInt(data.sale.total).toFixed(2)}`,
            7
          )
      );

      // Separator;
      printer
        .feed()
        .println("------------------------------------------------")
        .feed();

      // Payments;
      if (data.sale.payments && data.sale.payments.length) {
        data.sale.payments.forEach((payment) => {
          printer
            .println(
              this.addSpaces("Payment Method", 20) +
                this.addSpaces(payment.name, 10)
            )
            .println(
              this.addSpaces("Amount", 20) +
                this.addSpaces(
                  `${currency}${parseInt(payment.amount).toFixed(2)}`,
                  10
                )
            );
        });
        printer.feed(2);
      }
    }
  }

  // printReceiptFooter(printer, data) {
  //   printer.println(data.footerText);
  // }

  beginPrinting(printJobData: { id: string; message: string }) {
    const message = JSON.parse(printJobData.message);
    const data = JSON.parse(message.data);
    if (receiptProductId) {
      try {
        const device = new USB(receiptVendorId, receiptProductId);
        const options = { encoding: "GB18030" };
        const printer = new Printer(device, options);
        const logo = mp(`../../logos/logo-${resortLogo}.png`);
        Image.load(logo, (image) => {
          device.open(() => {
            this.printReceiptHeader(printer, image, data);
            if (message.printJobType === "receipt") {
              this.printReceiptBody(printer, data);
            } else {
              this.printReceiptCashoutBody(printer, data);
            }
            printer
              .feed(3)
              .cut()
              .close();

            // Give it at least 5 seconds to print before finishing the job
            setTimeout(() => {
              this.finishPrintJob(printJobData.id);
              console.log("Reciept printed");
            }, 300);
          });
        });
      } catch (e) {
        setTimeout(() => {
          this.finishPrintJob(printJobData.id);
          console.log("Reciept not printed");
        }, 300);
      }
    }
  }
}
export default ReceiptPrinter;
