const MessageQueue = require("../messageQueue");
const path = require("path");

//@ts-ignore
export function mp(relPath: string) {
  return path.resolve(__dirname, relPath);
}

export interface PrintJobMessage {
  id: string;
  message: string;
}
class NodePrint {
  mq: any;
  ws: any;
  qName: any;
  currentJobId: any;
  intervalId: any;
  currentState: any;
  numTimesCheckedPrinted: any;
  state: any;
  constructor(mq: any, ws: any, qName: any, state: any) {
    this.mq = mq;
    this.ws = ws;
    this.qName = qName;
    this.finishPrintJob = this.finishPrintJob;
    this.currentJobId = null;
    this.numTimesCheckedPrinted = 0;
    this.state = state;
  }

  /** Handle data, if no ID, end the Job */
  public print(printJobData: PrintJobMessage) {
    this.currentJobId = printJobData.id;
    const message = JSON.parse(printJobData.message);
    if (!message.id) {
      this.finishPrintJob(printJobData.id);
      return;
    } else {
    }

    /**start the print */
    this.beginPrinting(printJobData);
  }

  public beginPrinting(printJobData) {}

  /** Remove the message from the queue and remove id from local state */
  finishPrintJob(printJobId: string, holdUi = false) {
    this.mq.deleteMessage(this.qName, printJobId, (success: boolean) =>
      console.log(success)
    );

    this.currentJobId = null;
    if (!holdUi) {
      this.state.idle();
    }
  }

  /**Check every 500ms if a Job Is is set, then reseive a message from the specified Queue , Print it */
  public checkPrintJobs() {
    this.intervalId = setInterval(() => {
      if (
        this.currentJobId ||
        (this.state.currentState !== "idle" && this.qName !== "scanJobs")
      ) {
        return;
      }
      this.mq.receiveMessage(this.qName, (message) => {
        if (!message) {
          this.currentJobId = null;
          return;
        }
        this.state.updateState(this.qName);
        //@ts-ignore
        this.print(message);
      });
    }, 800);
  }

  public stop() {
    if (this.intervalId) {
      clearInterval(this.intervalId);
    }
  }
}

export default NodePrint;
