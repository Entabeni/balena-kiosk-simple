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

  beginPrinting(printJobData) {
    const message = JSON.parse(printJobData.message);

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
      }, 1500);
      setTimeout(() => {
        this.currentJobId = null;
        this.state.idle();
        this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
      }, 6500);
    });
  }
}

export default CardPrinter;
