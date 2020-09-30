import { v4 } from "uuid";
import fs from "fs";
import https from "https";
import NodePrinter, { mp } from "./NodePrint";
import CardRead from "./CardRead";
import printerMagicard from "node-native-printer";
printerMagicard.setPrinter("Magicard_600");
const pathCardPrint = require("path");

class CardPrinter extends NodePrinter {
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
    this.finishPrintJob = super.finishPrintJob;
    this.currentJobId = null;
    this.numTimesCheckedPrinted = 0;
    this.state = state;
  }
  downloadPDF(url, callback) {
    const directory = mp("../downloads/");
    fs.readdir(directory, (err, files) => {
      if (err) throw err;

      for (const file of files) {
        fs.unlink(pathCardPrint.join(directory, file), (err) => {
          if (err) throw err;
        });
      }
    });
    const filename = `${v4()}.pdf`;
    var file = fs.createWriteStream(mp(`../downloads/${filename}`));
    https.get(url, function(response) {
      response.pipe(file);
      file.on("finish", function() {
        file.close();
        callback(filename);
      });
    });
  }

  printingStarted(printJobData, jobId, message) {
    // Scan card
    const cardReader = new CardRead("ttyPrinterReader");

    let successfulScan = false;
    cardReader.startScanning();
    cardReader.readCardNumber((cardNumberHex) => {
      const last8Digits = cardNumberHex.substr(
        cardNumberHex.length - 8,
        cardNumberHex.length
      );

      this.ws.updateAccessRecord(message.accessRecordId, cardNumberHex);
      successfulScan = true;
      this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
      //@ts-ignore
      this.checkPrintingStoppedShort(jobId, message.id, successfulScan, false);
    });
    setTimeout(() => {
      if (!successfulScan) {
        this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
        this.checkPrintingStopped(jobId, message.id, successfulScan);
      }
    }, 6500);
  }

  /**
   * When the printing has stopped, set the print job to complete
   * And send success update
   */
  printingStopped(jobId, messageId, successfulScan, holdUi = false) {
    this.currentJobId = null;
    this.state.idle();
  }
  s;

  /**
   * Listen for when the printing starts
   * Loop every 500ms and when it starts, run printing started to scan card
   */
  checkPrintingStarted(printJobData, printString, message) {
    const jobId =
      printString &&
      printString.match(/-([0-9]+)/) &&
      printString.match(/-([0-9]+)/)[1];
    const printerInfo = printerMagicard.printerInfo("Magicard_600");
    const job = printerInfo.jobs.find((job) => job.id === jobId);
    this.numTimesCheckedPrinted++;
    if (
      ((job && (job.status === "processing" || job.status === "pending")) ||
        !job) &&
      this.numTimesCheckedPrinted === 27
    ) {
      this.numTimesCheckedPrinted = 0;
      this.finishPrintJob(printJobData.id);
      return;
    }

    if (
      (job && job.status === "processing") ||
      printerInfo.CUPSOptions["printer-state"] === "3"
    ) {
      this.numTimesCheckedPrinted = 0;
      setTimeout(() => {
        this.printingStarted(printJobData, jobId, message);
      }, 200);
    } else {
      setTimeout(
        () => this.checkPrintingStarted(printJobData, printString, message),
        500
      );
    }
  }

  /**
   * Listen for when the priting is over,
   * Loop every 500 ms, and when finished run printing stopped function
   */
  checkPrintingStopped(jobId, messageId, successfulScan) {
    setTimeout(() => {
      this.printingStopped(jobId, messageId, successfulScan, true);
    }, 7000);
  }
  checkPrintingStoppedShort(jobId, messageId, successfulScan) {
    setTimeout(() => {
      this.printingStopped(jobId, messageId, successfulScan, false);
    }, 10500);
  }

  /**
   * Use PDF in payload,
   * If no PDF, end the print job
   * Download PDF and print it in FITPLOT mode
   * and listen for when it starts
   */
  beginPrinting(printJobData) {
    const message = JSON.parse(printJobData.message);

    // Update print job status to processing

    const { pdfUrl } = JSON.parse(message.data);
    if (!pdfUrl) {
      this.finishPrintJob(printJobData.id);
      return;
    }

    this.downloadPDF(pdfUrl, (filename) => {
      setTimeout(() => {
        const options = { fitplot: true };
        const printString = printerMagicard.print(
          mp(`../downloads/${filename}`),
          options,
          "Magicard_600"
        );
        this.checkPrintingStarted(printJobData, printString, message);
      }, 1500);
    });
  }
}

export default CardPrinter;
