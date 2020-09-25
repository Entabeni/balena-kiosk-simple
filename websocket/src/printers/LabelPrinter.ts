import NodePrinter, { mp } from "./NodePrint";
import fs from "fs";
import printer from "node-native-printer";
const printerName = process.env.LABEL_PRINTER || "DYMO_LabelWriter_450";
printer.setPrinter(printerName);

const pathCard = require("path");
const printerSide = process.env.PRINTER_SIDE || "Left";

function splitIntoLines(input, len) {
  var i;
  var output = [];
  var lineSoFar = "";
  var temp;
  var words = input.split(" ");
  for (i = 0; i < words.length; ) {
    // check if adding this word would exceed the len
    temp = addWordOntoLine(lineSoFar, words[i]);
    if (temp.length > len) {
      if (lineSoFar.length == 0) {
        lineSoFar = temp; // force to put at least one word in each line
        i++; // skip past this word now
      }
      output.push(lineSoFar.replace(/,\s*$/, "").trim()); // put line into output
      lineSoFar = ""; // init back to empty
    } else {
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

class LabelPrinter extends NodePrinter {
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

  beginPrinting(printJobData: { id: string; message: string }) {
    const message = JSON.parse(printJobData.message);

    const addressJson = JSON.parse(JSON.parse(message.data));
    const {
      purchaserName,
      address1,
      address2,
      state,
      zip,
      country,
      city,
    } = addressJson;
    let addressLine = address2 ? address1 + ", " + address2 : address1;
    let addressLineSplit = splitIntoLines(addressLine, 27);
    let cityStateZip = splitIntoLines(city + ", " + state + ", " + zip, 27);
    let cityStateZipBetween = cityStateZip.length > 1 ? ", " : ",\n";
    function mpTest(relPath) {
      return pathCard.resolve(__dirname, relPath);
    }
    fs.unlink(mpTest("./log.txt"), () => {
      const finalString =
        purchaserName +
        ",\n" +
        addressLineSplit.join(",\n") +
        ",\n" +
        cityStateZip.join(",\n") +
        cityStateZipBetween +
        country;
      fs.appendFile(mpTest("./log.txt"), finalString, (err) => {
        if (err) {
          this.finishPrintJob(printJobData.id);
        } else {
          printer.print(mpTest("./log.txt"), {
            landscape: true,
            InputSlot: printerSide,
            pageSize: "24_mm__1___Label__Auto_",
          });
          this.finishPrintJob(printJobData.id);
        }
      });
    });
  }
}
export default LabelPrinter;
