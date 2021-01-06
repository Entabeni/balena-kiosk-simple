import { PrintJobQueryData } from "../ws";

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
  ws: any;
  qName: any;
  currentJobId: any;
  intervalId: any;
  currentState: any;
  numTimesCheckedPrinted: any;
  constructor(ws: any, qName: any) {
    this.ws = ws;
    this.qName = qName;
    this.currentJobId = null;
    this.numTimesCheckedPrinted = 0;
  }

  /** Handle data, if no ID, end the Job */
  public print(printJobData: PrintJobQueryData) {
    /**start the print */
    this.beginPrinting(printJobData);
  }

  public beginPrinting(printJobData) {}

  /** Remove the message from the queue and remove id from local state */

  /**Check every 500ms if a Job Is is set, then reseive a message from the specified Queue , Print it */
  // public checkPrintJobs() {
  //   this.intervalId = setInterval(() => {
  //     if (
  //       this.currentJobId ||
  //       (this.state.currentState !== "idle" && this.qName !== "scanJobs")
  //     ) {
  //       return;
  //     }
  //     this.mq.receiveMessage(this.qName, (message) => {
  //       if (!message) {
  //         this.currentJobId = null;
  //         return;
  //       }
  //       this.state.updateState(this.qName);
  //       //@ts-ignore
  //       this.print(message);
  //     });
  //   }, 1200);
  // }

  public stop() {
    if (this.intervalId) {
      clearInterval(this.intervalId);
    }
  }
}

export default NodePrint;
