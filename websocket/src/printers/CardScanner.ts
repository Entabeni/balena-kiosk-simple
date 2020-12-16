import NodePrinter, { mp } from "./NodePrint";
import CardRead from "./CardRead";

class CardScanner extends NodePrinter {
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
  /**
   * If no message ID, end the job
   * Else send update that the scan job is processing
   * Begin printing
   */
  print(printJobData) {
    this.currentJobId = printJobData.id;
    const message = JSON.parse(printJobData.message);
    if (!message.id) {
      this.finishPrintJob(printJobData.id);
      return;
    }
    if (message.status === "cashDrawerOpen") {
      const cashPort = new SerialPort("/dev/ttyCashDrawer", { baudRate: 9600 });

      cashPort.write("open", () => {
        cashPort.close();
      });

      this.mq.deleteMessage(this.qName, printJobData.id, (success) =>
        console.log("deleted message", success)
      );
      this.ws.updateScanJob(message.id, "cashDrawerOpenCompleted");
      this.currentJobId = null;
      this.state.idle();
      return;
    }
    // Update print job status to processing
    // this.ws.updateScanJob(message.id, "processing");
    this.openScanner(printJobData);
  }

  openScanner(printJobData) {
    const message = JSON.parse(printJobData.message);

    const cardReader = new CardRead("ttyDesktopReader");
    let successfulScan = false;
    cardReader.startScanning();
    cardReader.readCardNumber((cardNumberHex: string) => {
      if (this.currentState !== "idle") {
        const last8Digits = cardNumberHex.substr(
          cardNumberHex.length - 8,
          cardNumberHex.length
        );
        if (message && message.accessRecordId) {
          this.ws.updateAccessRecord(message.accessRecordId, cardNumberHex);
        }
        successfulScan = true;
        this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
        this.ws.updateScanJob(message.id, "completed", cardNumberHex);
        this.currentJobId = null;
        this.state.idle();
      }
    });
    setTimeout(() => {
      if (!successfulScan) {
        this.mq.deleteMessage(this.qName, printJobData.id, (success) => {});
        this.currentJobId = null;
        this.state.idle();
      }
    }, 9000);
  }
}

export default CardScanner;
