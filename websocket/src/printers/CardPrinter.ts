import { PrintJobQueryData } from "./../ws";
// @ts-ignore
import { v4 } from "uuid";
import fs from "fs";
import https from "https";
import NodePrinter, { mp } from "./NodePrint";
import CardRead from "./CardRead";
import printerMagicard from "node-native-printer";
printerMagicard.setPrinter("Magicard_600");
const pathCardPrint = require("path");

class CardPrinter extends NodePrinter {
  ws: any;
  qName: any;
  currentJobId: any;
  intervalId: any;
  currentState: any;
  numTimesCheckedPrinted: any;
  state: any;
  constructor(ws: any, qName: any) {
    super(ws, qName);
    this.ws = ws;
    this.qName = qName;
    this.currentJobId = null;
    this.numTimesCheckedPrinted = 0;
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
    this.currentJobId = null;
    this.state.idle();
  }

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
   * Use PDF in payload,
   * If no PDF, end the print job
   * Download PDF and print it in FITPLOT mode
   * and listen for when it starts
   */
  beginPrinting(printJobData: PrintJobQueryData) {
    const message = JSON.parse(printJobData.pos.printJob.printData);

    // Update print job status to processing

    const pdfUrl = message.pdfUrl;

    if (!pdfUrl) {
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
        // this.checkPrintingStarted(printJobData, printString, message);
      }, 1500);
    });
  }
}

export default CardPrinter;
